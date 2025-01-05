

#include "HDSDK.h"

#include <functional>
#include <string>
#include <list>
#include <limits>
#include <vector>
#include <memory>


#define VERSION                     "1.0.0.0"


#define MD5_LENGHT                  (32)
#define XML_MAX                     (9200)
#define LOCAL_TCP_VERSION           (0x1000009)
#define LOCAL_UDP_VERSION       	(0x1000009)


typedef unsigned long long      huint64;
typedef unsigned int            huint32;
typedef unsigned short          huint16;
typedef unsigned char           huint8;

typedef long long               hint64;
typedef int                     hint32;
typedef short                   hint16;
typedef char                    hint8;
typedef float                   hfloat;
typedef double                  hdouble;



namespace detail
{


enum eHCmdType
{
    kTcpHeartbeatAsk        = 0x005f,   ///< TCP心跳包请求
    kTcpHeartbeatAnswer     = 0x0060,   ///< TCP心跳包反馈
    kSearchDeviceAsk        = 0x1001,   ///< 搜索设备请求
    kSearchDeviceAnswer     = 0x1002,   ///< 搜索设备应答
    kErrorAnswer            = 0x2000,   ///< 出错反馈

    kSDKServiceAsk          = 0x2001,   ///< 版本协商请求
    kSDKServiceAnswer       = 0x2002,   ///< 版本协商应答
    kSDKCmdAsk              = 0x2003,   ///< sdk命令请求
    kSDKCmdAnswer           = 0x2004,   ///< sdk命令反馈
    kFileStartAsk           = 0x8001,   ///< 文件开始传输请求
    kFileStartAnswer        = 0x8002,   ///< 文件开始传输应答
    kFileContentAsk         = 0x8003,   ///< 携带文件内容的请求
    kFileContentAnswer      = 0x8004,   ///< 写文件内容的应答
    kFileEndAsk             = 0x8005,   ///< 文件结束传输请求
    kFileEndAnswer          = 0x8006,   ///< 文件结束传输应答
    kReadFileAsk            = 0x8007,   ///< 回读文件请求
    kReadFileAnswer         = 0x8008,   ///< 回读文件应答
};


template <typename _Func>
class HCallBack
{
public:
    HCallBack()
        : userData_(nullptr)
    {}

    void Bind(const std::function<_Func> &func) {
        callBack_ = func;
    }

    void ClearFunc() { callBack_ = std::function<_Func>();}

    void SetUserData(void *userData) {
        userData_ = userData;
    }

    template <typename ..._Args>
#if __cplusplus >= 201402L
    decltype(auto)
#else
    typename std::result_of<std::function<_Func>(_Args..., void *)>::type
#endif
    Emit(_Args &&...args) {
        auto func(callBack_);
        return func(std::forward<_Args>(args)..., userData_);
    }

    template <typename ..._Args>
#if __cplusplus >= 201402L
    decltype(auto)
#else
    typename std::result_of<std::function<_Func>(_Args..., void *)>::type
#endif
    operator()(_Args &&...args) {
        return this->Emit(std::forward<_Args>(args)...);
    }

    bool IsCanUse() const { return bool(callBack_); }

private:
    std::function<_Func> callBack_;
    void *userData_;
};


class HAny
{
private:
    struct DataBase;
    using DataBasePtrType = std::shared_ptr<DataBase>;
    struct DataBase{
        virtual ~DataBase() {}
        virtual DataBasePtrType Clone() const = 0;
        virtual const std::type_info &Type() const = 0;
    };

    template<typename _T>
    struct Data : public DataBase
    {
        template<typename...Args>
        Data(Args&&...args) : data(std::forward<Args>(args)...){ }

        virtual DataBasePtrType Clone() const override { return DataBasePtrType(new Data(data)); }
        virtual const std::type_info &Type() const override { return typeid(_T); }

        _T data;
    };

    DataBasePtrType Clone() const {
        if (data_) {
            return data_->Clone();
        }
        return nullptr;
    }
public:
    HAny() {}
    HAny(const HAny &other) : data_(other.data_) {}
    template <typename _T, typename = typename std::enable_if<!std::is_same<typename std::decay<_T>::type, HAny>::value, _T>::type>
    HAny(_T &&t) : data_(new Data<typename std::decay<_T>::type>(std::forward<_T>(t))) {}

    const std::type_info &Type() const { return data_ ? data_->Type() : typeid(void); }

    template <typename _T>
    bool IsType() const { return Type() == typeid(_T); }

    template <typename _T>
    _T& Cast() const {
        Detach();
        auto p = static_cast<Data<typename std::decay<_T>::type>*>(this->data_.get());
        return p->data;
    }

private:
    void Detach() const {
        if (!data_ || data_.unique()) {
            return ;
        }

        data_ = data_->Clone();
    }

private:
    mutable DataBasePtrType data_;
};


template <typename _T>
std::string IconvStr(_T value) {
    std::string ret;
    for (std::size_t i = 0; i < sizeof(_T); ++i) {
        ret.push_back(char(value & 0xff));
        value = value >> 8;
    }
    return ret;
}

template <typename _T>
_T StrIconv(const char *value, std::size_t len = std::numeric_limits<std::size_t>::max()) {
    _T ret = _T();
    for (std::size_t i = 0; i < sizeof(typename std::remove_cv<_T>::type) && i < len; ++i) {
        ret = ret | ((value[i] & 0xff) << (8 * i));
    }
    return ret;
}

template <typename _T>
_T StrIconv(const std::string &value) { return StrIconv<_T>(value.data(), value.size()); }


static std::list<std::string> SplitSdkData(const std::string &xml)
{
    std::list<std::string> queue;
    int size = static_cast<int>(xml.size());
    huint32 index = 0;

    for (; size > XML_MAX; size -= XML_MAX, index += XML_MAX){
        std::string data = IconvStr<huint16>(12 + XML_MAX);
        data.append(IconvStr<huint16>(kSDKCmdAsk));
        data.append(IconvStr<huint32>(xml.size()));
        data.append(IconvStr<huint32>(index));
        data.append(xml.data() + index, XML_MAX);

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        std::string data = IconvStr<huint16>(12 + static_cast<huint16>(size));
        data.append(IconvStr<huint16>(kSDKCmdAsk));
        data.append(IconvStr<huint32>(xml.size()));
        data.append(IconvStr<huint32>(index));
        data.append(xml.data() + index, size);

        queue.emplace_back(std::move(data));
    }

    return queue;
}

static std::list<std::string> SplitFileData(const char *fileData, int size)
{
    std::list<std::string> queue;
    huint32 index = 0;

    for (; size > XML_MAX; size -= XML_MAX, index += XML_MAX){
        std::string data = IconvStr<huint16>(4 + XML_MAX);
        data.append(IconvStr<huint16>(kFileContentAsk));
        data.append(fileData + index, XML_MAX);

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        std::string data = IconvStr<huint16>(4 + static_cast<huint16>(size));
        data.append(IconvStr<huint16>(kFileContentAsk));
        data.append(fileData + index, size);

        queue.emplace_back(std::move(data));
    }

    return queue;
}


static std::string &Remove(std::string &buff, std::size_t index, std::size_t num = std::string::npos) {
    if (index > buff.size()) {
        index = buff.size();
    }

    return buff.erase(index, num);
}

static std::string Mid(const std::string &buff, std::size_t index, std::size_t len = std::string::npos) {
    if (index >= buff.size()) {
        return std::string();
    }

    if (len > buff.size()) {
        len = buff.size();
    }

    if (len > buff.size() - index) {
        len = buff.size() - index;
    }

    if (index == 0 && len == buff.size()) {
        return buff;
    }

    return buff.substr(index, len);
}


static std::string GetXmlAttribute(const std::string &xml, const std::string &node, const std::string &attributeName) {
    std::string nodeName = "<" + node;
    std::string::size_type pos = xml.find(nodeName);
    if (pos == std::string::npos) {
        return "";
    }

    pos = xml.find(attributeName, pos + nodeName.size());
    if (pos == std::string::npos) {
        return "";
    }

    pos = xml.find('"', pos + attributeName.size());
    if (pos == std::string::npos) {
        return "";
    }

    ++pos;
    std::string::size_type posEnd = xml.find('"', pos);
    if (posEnd == std::string::npos) {
        return "";
    }

    return xml.substr(pos, posEnd - pos);
}

static std::string SetXmlAttribute(const std::string &xml, const std::string &node, const std::string &attributeName, const std::string &attributeValue) {
    std::string nodeName = "<" + node;
    std::string::size_type pos = xml.find(nodeName);
    if (pos == std::string::npos) {
        return xml;
    }

    pos = xml.find(attributeName, pos + nodeName.size());
    if (pos == std::string::npos) {
        return xml;
    }

    pos = xml.find('"', pos + attributeName.size());
    if (pos == std::string::npos) {
        return xml;
    }

    ++pos;
    std::string::size_type posEnd = xml.find('"', pos);
    if (posEnd == std::string::npos) {
        return xml;
    }

    std::string result = xml;
    return result.replace(pos, posEnd - pos, attributeValue);
}


struct DelaySend {
    int cmd;
    std::string data;
    detail::HAny userData;
    DelaySend()
        : cmd(0)
    {}

    DelaySend(int c, const std::string &d, const detail::HAny &u)
        : cmd(c)
        , data(d)
        , userData(u)
    {}
    DelaySend(int c, std::string &&d, detail::HAny &&u)
        : cmd(c)
        , data(std::move(d))
        , userData(std::move(u))
    {}
};


struct SendFileInfo {
    hint64 index;
    hint64 size;
    hint16 type;
    std::string md5;
    std::string name;
    std::function<int(hint64 index, hint64 size, int status, char *buff, int buffSize, void *userData)> call;
    void *userData;

    SendFileInfo()
        : index(0)
        , size(0)
        , type(0)
        , userData(nullptr)
    {}

    void Clear() {
        index = 0;
        size = 0;
        type = 0;
        md5.clear();
        name.clear();
        call = std::function<int(hint64, hint64, int, char *, int, void *)>();
        userData = nullptr;
    }
};


///< 绑定成员函数的回调
template <typename _Ret, typename _Obj, typename... _Args>
std::function<_Ret(_Args...)> BindMember(_Obj *obj, _Ret(_Obj::*func)(_Args...))
{
    return [&](_Args &&...args) {
        return (obj->*func)(std::forward<_Args>(args)...);
    };
}

///< 绑定普通函数的回调
template <typename _Ret, typename... _Args>
std::function<_Ret(_Args...)> BindFunc(_Ret(*func)(_Args...))
{
    return [&](_Args &&...args) {
        return func(std::forward<_Args>(args)...);
    };
}


}


namespace hd
{


///< SDK私有数据类, 用于保留接口一致
class HDSDKPrivate
{
public:
    detail::HCallBack<void(const char *xml, int len, int errorCode, void *userData)> NotifyReadXml;
    detail::HCallBack<bool(const char *data, int len, void *userData)> NotifySendData;
    detail::HCallBack<int(hint64 index, hint64 size, int status, char *buff, int buffSize, void *userData)> NotifySendFile;

public:
    enum eCmd {
        kInitRun,   ///< 初始化协商
        kReadData,  ///< 读取数据
        kSendSDK,   ///< 发送xml
        kSendFile,  ///< 发送文件
    };

public:
    HDSDKPrivate() { Init(); }

    void Init();
    bool Dispose(int cmd, const std::string &data, const detail::HAny &userData = detail::HAny());

private:
    bool ParseReadData(const std::string &data);
    bool ParseNegotiate(int num);
    bool ParseSDKCmdAnswer(std::string &&data);
    bool ParseSendSDK(int cmd, const std::string &data, const detail::HAny &userData);
    bool ParseFileStartAnswer(std::string &&buff);
    bool SendFileEnd();
    bool DelaySend();

    ///< 通知回调接口处理任务
    bool SendData(const std::string &data) { return SendData(data.c_str(), data.size()); }
    bool SendData(const char *data, int len);
    void ReadXml(const std::string &xml, int errorCode);
    int SendFile(hint64 index, hint64 size, int status, char *buff, int buffSize);

private:
    friend class HDSDK;
    std::string guid_;                              ///< 通信的guid
    std::string sdkBuff_;                           ///< sdk xml缓存
    std::string readBuff_;                          ///< 总数据缓存
    std::list<detail::DelaySend> delaySendList_;    ///< 延迟发送队列
    detail::SendFileInfo fileInfo_;                 ///< 发送文件信息
    bool negotiate_;                                ///< 协商状态
    bool processing_;                               ///< 发送文件中独占
};

void HDSDKPrivate::Init()
{
    guid_.clear();
    negotiate_ = false;
    processing_ = false;
    delaySendList_.clear();
    std::string().swap(sdkBuff_);
    std::string().swap(readBuff_);
}

bool HDSDKPrivate::Dispose(int cmd, const std::string &data, const detail::HAny &userData)
{
    switch (cmd) {
    case kInitRun: {
        return ParseNegotiate(0);
    } break;
    case kReadData: {
        return ParseReadData(data);
    } break;
    default:
        break;
    }

    if (processing_) {
        delaySendList_.emplace_back(detail::DelaySend(cmd, data, userData));
        return true;
    }

    switch (cmd) {
    case kSendSDK: {
        return ParseSendSDK(cmd, data, userData);
    } break;
    case kSendFile: {
        if (negotiate_ == false) {
            delaySendList_.emplace_back(detail::DelaySend(cmd, data, userData));
            return true;
        }

        if (userData.IsType<detail::SendFileInfo>() == false) {
            return false;
        }

        fileInfo_ = userData.Cast<detail::SendFileInfo>();
        if (fileInfo_.size <= 0 || fileInfo_.md5.empty() || fileInfo_.name.empty()) {
            return false;
        }

        if (fileInfo_.md5.size() < MD5_LENGHT) {
            fileInfo_.md5.append(MD5_LENGHT - fileInfo_.md5.size(), '\0');
        }

        NotifySendFile.Bind(fileInfo_.call);
        NotifySendFile.SetUserData(fileInfo_.userData);

        std::string sendData = detail::IconvStr<huint16>(static_cast<huint16>(47 + fileInfo_.name.size() + 1));
        sendData.append(detail::IconvStr<huint16>(detail::kFileStartAsk));
        sendData.append(fileInfo_.md5);
        sendData.append(1, '\0');
        sendData.append(detail::IconvStr<huint64>(fileInfo_.size));
        sendData.append(detail::IconvStr<hint16>(fileInfo_.type));
        sendData.append(fileInfo_.name);
        sendData.append(1, '\0');
        if (SendData(sendData) == false) {
            return false;
        }

        processing_ = true;
        return true;
    } break;
    default:
        break;
    }

    return false;
}

bool HDSDKPrivate::ParseReadData(const std::string &data)
{
    if (data.empty() == false) {
        readBuff_.append(data);
    }

    if (readBuff_.size() < 4) {
        return true;
    }

    std::size_t len =  detail::StrIconv<huint16>(readBuff_.c_str(), readBuff_.size());
    int cmd =  detail::StrIconv<huint16>(readBuff_.c_str() + 2, readBuff_.size() - 2);
    if (len > readBuff_.size()) {
        return true;
    }

    if (len == 0) {
        return false;
    }

    std::string buff = detail::Mid(readBuff_, 0, len);
    detail::Remove(readBuff_, 0, len);

    bool result = true;
    switch (cmd) {
    case detail::kTcpHeartbeatAsk:
    case detail::kTcpHeartbeatAnswer: {
        std::string sendData = detail::IconvStr<huint16>(4);
        sendData.append(detail::IconvStr<huint16>(detail::kTcpHeartbeatAsk));
        result = SendData(sendData);
    } break;
    case detail::kSDKServiceAnswer: {
        result = ParseNegotiate(1);
    } break;
    case detail::kSDKCmdAnswer: {
        result = ParseSDKCmdAnswer(std::move(buff));
    } break;
    case detail::kErrorAnswer: {
        if (buff.size() < 6) {
            return false;
        }
        ReadXml("", detail::StrIconv<huint16>(buff.c_str() + 4, buff.size() - 4));
    } break;
    case detail::kFileStartAnswer: {
        result = ParseFileStartAnswer(std::move(buff));
    } break;
    case detail::kFileContentAnswer:
        break;
    case detail::kFileEndAnswer: {
        SendFile(fileInfo_.index, fileInfo_.size, 0, nullptr, 0);
        processing_ = false;
        result = DelaySend();
    } break;
    default:
        break;
    }

    if (result && readBuff_.empty() == false) {
        return ParseReadData("");
    }

    return result;
}

bool HDSDKPrivate::ParseNegotiate(int num)
{
    std::string data;
    if (num == 0) {
        data.append(detail::IconvStr<huint16>(8));
        data.append(detail::IconvStr<huint16>(detail::kSDKServiceAsk));
        data.append(detail::IconvStr<huint32>(LOCAL_TCP_VERSION));
    } else {
        std::string xml(R"(<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
      <in method="GetIFVersion">
        <version value="1000000"/>
      </in>
</sdk>)");

        data = detail::SplitSdkData(xml).front();
    }

    return SendData(data);
}

bool HDSDKPrivate::ParseSDKCmdAnswer(std::string &&data)
{
    if (data.size() < 12) {
        return false;
    }

//    int len = detail::StrIconv<huint16>(data.c_str());
//    int cmd = detail::StrIconv<huint16>(data.c_str() + 2, data.size() - 2);
    huint32 total = detail::StrIconv<huint32>(data.c_str() + 4, data.size() - 4);
//    int index = detail::StrIconv<huint32>(data.c_str() + 8, data.size() - 8);
    detail::Remove(data, 0, 12);

    if (negotiate_ == false) {
        guid_ = detail::GetXmlAttribute(data, "sdk", "guid");
        negotiate_ = true;
        return DelaySend();
    }

    if (data.empty()) {
        return true;
    }

    sdkBuff_.append(data);
    if (total <= sdkBuff_.size()) {
        std::string buff(std::move(sdkBuff_));
        sdkBuff_.clear();
        ReadXml(buff, 0);
    }

    return true;
}

bool HDSDKPrivate::ParseSendSDK(int cmd, const std::string &data, const detail::HAny &userData)
{
    if (negotiate_ == false) {
        delaySendList_.emplace_back(detail::DelaySend(cmd, data, userData));
        return true;
    }

    if (data.empty()) {
        return true;
    }

    std::string xml = detail::SetXmlAttribute(data, "sdk", "guid", guid_);
    auto xmlList = detail::SplitSdkData(xml);
    for (const auto &i : xmlList) {
        if (SendData(i) == false) {
            return false;
        }
    }

    return true;
}

bool HDSDKPrivate::ParseFileStartAnswer(std::string &&buff)
{
    if (buff.size() < 14) {
        return false;
    }

    processing_ = true;
    huint16 status = detail::StrIconv<huint16>(buff.c_str() + 4, buff.size() - 4);
    fileInfo_.index = detail::StrIconv<hint64>(buff.c_str() + 6, buff.size() - 6);
    if (status != 0) {
        SendFile(fileInfo_.index, fileInfo_.size, status, nullptr, 0);
        processing_ = false;
        return DelaySend();
    }

    std::vector<char> sendBuff(XML_MAX, '\0');
    while (fileInfo_.index < fileInfo_.size) {
        int len = SendFile(fileInfo_.index, fileInfo_.size, status, sendBuff.data(), sendBuff.size());
        if (len <= 0) {
            processing_ = false;
            return false;
        }

        fileInfo_.index += len;
        auto sendList = detail::SplitFileData(sendBuff.data(), len);
        for (const auto &i : sendList) {
            if (SendData(i) == false) {
                processing_ = false;
                return false;
            }
        }
    }

    return SendFileEnd();
}

bool HDSDKPrivate::SendFileEnd()
{
    std::string data = detail::IconvStr<huint16>(4);
    data.append(detail::IconvStr<huint16>(detail::kFileEndAsk));
    return SendData(data);
}

bool HDSDKPrivate::DelaySend()
{
    std::list<detail::DelaySend> sendList;
    sendList.swap(delaySendList_);
    while (sendList.empty() == false) {
        if (processing_) {
            delaySendList_.splice(delaySendList_.begin(), std::move(sendList));
            break;
        }

        detail::DelaySend i = std::move(sendList.front());
        sendList.pop_front();
        if (Dispose(i.cmd, i.data, i.userData) == false) {
            delaySendList_.clear();
            return false;
        }
    }

    return true;
}

bool HDSDKPrivate::SendData(const char *data, int len)
{
    if (NotifySendData.IsCanUse() == false) {
        return false;
    }

    return NotifySendData(data, len);
}

void HDSDKPrivate::ReadXml(const std::string &xml, int errorCode)
{
    if (NotifyReadXml.IsCanUse() == false) {
        return ;
    }
    NotifyReadXml(xml.c_str(), xml.size(), errorCode);
}

int HDSDKPrivate::SendFile(hint64 index, hint64 size, int status, char *buff, int buffSize)
{
    if (NotifySendFile.IsCanUse() == false) {
        return -1;
    }

    return NotifySendFile(index, size, status, buff, buffSize);
}


}


const char *Version()
{
    return VERSION;
}


IHDProtocol CreateProtocol()
{
    return new hd::HDSDKPrivate();
}

void FreeProtocol(IHDProtocol protocol)
{
    delete protocol;
}

HBool SetProtocolFunc(IHDProtocol protocol, int func, void *data)
{
    if (protocol == nullptr) {
        return HFalse;
    }

    switch (func) {
    case kSetReadXml: {
        if (data) {
            protocol->NotifyReadXml.Bind(reinterpret_cast<void(*)(const char *xml, int len, int errorCode, void *userData)>(data));
        } else {
            protocol->NotifyReadXml.ClearFunc();
        }
    } break;
    case kSetReadXmlData: {
        protocol->NotifyReadXml.SetUserData(data);
    } break;
    case kSetSendFunc: {
        if (data) {
            protocol->NotifySendData.Bind(reinterpret_cast<bool(*)(const char *data, int len, void *userData)>(data));
        } else {
            protocol->NotifySendData.ClearFunc();
        }
    } break;
    case kSetSendFuncData: {
        protocol->NotifySendData.SetUserData(data);
    } break;
    default:
        return HFalse;
        break;
    }

    return HTrue;
}

void InitProtocol(hd::IHDProtocol protocol)
{
    protocol->Init();
}

HBool RunProtocol(hd::IHDProtocol protocol)
{
    return protocol->Dispose(hd::HDSDKPrivate::kInitRun, "") ? HTrue : HFalse;
}

HBool UpdateReadData(hd::IHDProtocol protocol, const char *data, int len)
{
    return protocol->Dispose(hd::HDSDKPrivate::kReadData, std::string(data, len)) ? HTrue : HFalse;
}

HBool SendXml(hd::IHDProtocol protocol, const char *xml, int len)
{
    return protocol->Dispose(hd::HDSDKPrivate::kSendSDK, std::string(xml, len)) ? HTrue : HFalse;
}

HBool SendFile(hd::IHDProtocol protocol, const char *fileName, int fileNameLen, const char *md5, int type, hint64 size, int (*callFun)(hint64, hint64, int, char *, int, void *), void *userData)
{
    detail::SendFileInfo info;
    info.size = size;
    info.type = type;
    info.md5.assign(md5);
    info.name.assign(fileName, fileNameLen);
    info.call = callFun;
    info.userData = userData;

    if (info.md5.empty() || info.md5.size() != MD5_LENGHT) {
        return HFalse;
    }

    if (info.name.empty()) {
        return HFalse;
    }

    return protocol->Dispose(hd::HDSDKPrivate::kSendFile, "", info) ? HTrue : HFalse;
}
