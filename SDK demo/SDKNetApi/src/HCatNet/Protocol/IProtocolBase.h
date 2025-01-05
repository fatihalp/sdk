

#ifndef __IPROTOCOLBASE_H__
#define __IPROTOCOLBASE_H__


#include <HCatAny.h>
#include <HCatNetInfo.h>
#include <HCatSignal.h>
#include <ICatEventApi.h>
#include <ICatEventData.h>
#include <HCatBuffer.h>

#include <numeric>
#include <algorithm>
#include <utility>


#define PROGRESS_TIMEOUT            (1000)
#define IO_DELAY_TIME               (1)
#define IO_SLEEP_NUM                (50)


namespace cat
{


///< 发送文件传递给协议的数据
struct HSendProtocolFileInfo
{
    SendFileListType fileList;
    HCatAny userData;

    void Clear() { fileList.clear(); userData.Clear(); }
    HSendProtocolFileInfo() {}
    explicit HSendProtocolFileInfo(const SendFileListType &fileList_) : fileList(fileList_) {}
    explicit HSendProtocolFileInfo(const SendFileListType &fileList_, const HCatAny &userData_) : fileList(fileList_), userData(userData_) {}
};

///< 读取文件传递给协议的数据
struct HReadProtocolFileInfo
{
    ReadFileListType fileList;
    HCatAny userData;

    void Clear() { fileList.clear(); userData.Clear(); }
    HReadProtocolFileInfo() {}
    explicit HReadProtocolFileInfo(const ReadFileListType &fileList_) : fileList(fileList_) {}
    explicit HReadProtocolFileInfo(const ReadFileListType &fileList_, const HCatAny &userData_) : fileList(fileList_), userData(userData_) {}
};


///< 发送文件的流计算
struct HFlowFile
{
public:
    std::string path;   ///< 文件路径
    hint64 fileSize;    ///< 文件大小
    hint64 flowSize;    ///< 封包后的总大小
    hint32 headSize;    ///< 每包的头大小
    hint32 packSize;    ///< 每包的最大大小
    hint32 roundSize;   ///< 每轮打包大小, 使用多少来计算每包

public:
    HFlowFile() : fileSize(0), flowSize(0), headSize(0), packSize(0), roundSize(0) {}
    HFlowFile(const std::string &filePath, hint64 fileSize_) : path(filePath), fileSize(fileSize_), headSize(0), packSize(0), roundSize(0) { }

    bool operator ==(const HFlowFile &other) const { return path == other.path && fileSize == other.fileSize; }

    ///< 修改文件大小, 重计算流大小
    HFlowFile &SetFileSize(hint64 size) { fileSize = size; return Adjust(); }
    ///< 设置流文件后的大小, 传入包头大小和每包大小
    HFlowFile &SetFlowSize(hint32 headSize_, hint32 packSize_, hint32 roundSize_) { headSize = headSize_; packSize = packSize_; roundSize = roundSize_; return Adjust(); }

    static hint64 CalcFlowSize(hint32 headSize, hint32 packSize, hint64 fileSize) {
        hint64 packCount = fileSize / packSize;
        if (fileSize % packSize) {
            ++packCount;
        }
        return packCount * headSize + fileSize;
    }

    static hint64 CalcFlowSize(hint32 headSize, hint32 packSize, hint32 roundSize, hint64 fileSize) {
        if (roundSize <= 0) {
            return CalcFlowSize(headSize, packSize, fileSize);
        }

        hint64 packCount = fileSize / roundSize;
        return CalcFlowSize(headSize, packSize, roundSize) * packCount + CalcFlowSize(headSize, packSize, fileSize % roundSize);
    }

    HFlowFile &Adjust() {
        if (packSize <= 0) {
            flowSize = headSize + fileSize;
            return *this;
        }

        flowSize = CalcFlowSize(headSize, packSize, roundSize, fileSize);
        return *this;
    }

    ///< 计算当前发送流大小对应文件的大小
    hint64 CalcCurrSendFileSize(hint64 sendFlow) const {
        if (sendFlow >= flowSize) {
            return fileSize;
        }
        hint64 frameSize = headSize + packSize;
        hint64 packCount = sendFlow / frameSize;
        return packSize * packCount + sendFlow % frameSize;
    }
};

///< 发送流文件接口
struct HFlowFileList
{
private:
    hint64 total;       ///< 流的总量
    hint64 adjustFlow;  ///< 流的校准值(实际不存在的流量), 断点续传需要校准

public:
    hint64 headSize;    ///< 一些杂余的头包占据的流量(实际存在的流量)
    std::list<HFlowFile> flowList;  ///< 发送的文件流

public:
    struct FilePack
    {
        std::string path;
        hint64 send;
        hint64 size;
        FilePack() : send(0), size(0) {}
        FilePack(const std::string &path_, hint64 send_, hint64 size_) : path(path_), send(send_), size(size_) {}

        bool operator ==(const FilePack &other) const { return path == other.path && size == other.size; }
        bool operator !=(const FilePack &other) const { return !(*this == other); }
    };

public:
    hint64 FlowTotal() const { return total + headSize; }
    hint64 SendFlow(hint64 send) const { return send + adjustFlow; }

    HFlowFileList() : total(0), adjustFlow(0), headSize(0) {}

    ///< 校准所有文件流的流大小
    HFlowFileList &AdjustFlowList() {
        for (auto &i : flowList) {
            i.Adjust();
        }
        return *this;
    }

    ///< 增加校准流值
    HFlowFileList &AddAdjustFlow(hint64 dataSize, hint32 headSize = 0, hint32 packSize = 0, hint32 roundSize = 0) {
        if (packSize > 0) {
            adjustFlow += HFlowFile::CalcFlowSize(headSize, packSize, roundSize, dataSize);
        } else {
            adjustFlow += dataSize + headSize;
        }
        return *this;
    }

    ///< 校准完文件流大小后校准总量
    HFlowFileList &AdjustTotal() { total = std::accumulate(flowList.begin(), flowList.end(), hint64(0), [](hint64 v, const HFlowFile &f) { return v + f.flowSize; }); return *this; }

    ///< 判断是否已经达到结束点
    bool IsFinish(hint64 sendFlow) const { return SendFlow(sendFlow) >= FlowTotal(); }

    FilePack GetLastFilePack() const {
        if (flowList.empty() == false) {
            const auto &flow = flowList.back();
            return FilePack(flow.path, flow.fileSize, flow.fileSize);
        }

        return FilePack();
    }

    ///< 获取流对应的发送文件大小和文件大小
    FilePack GetFilePack(hint64 sendFlow) const {
        sendFlow = SendFlow(sendFlow);
        if (sendFlow >= FlowTotal()) {
            return GetLastFilePack();
        }

        sendFlow -= headSize;
        for (const auto &i : flowList) {
            if (sendFlow < i.flowSize) {
                return FilePack{i.path, i.CalcCurrSendFileSize(sendFlow), i.fileSize};
            }
            sendFlow -= i.flowSize;
        }

        return GetLastFilePack();
    }
};


class IProtocolBase : public HCatEventObject
{
public:
    ///< 生存测试指针, 用于传递线程事件时提前析构时能判断是否需要等待任务退出
    using survivalTest_type = bool;
    using survivalTest_typePtr = std::shared_ptr<survivalTest_type>;
    using survivalTestWear_typePtr = std::weak_ptr<survivalTest_type>;

public:
    ///< 传输给协议处理的指令
    enum eProtocolCmd {
        kInit,                  ///< 要求初始化全部数据, 通知重置后在Disponse返回false时需要重新初始化
        kReset,                 ///< 在返回false时, 初始化完成后将通知重新协商协议
        kSendFaildAsk,          ///< 通知协议发送失败
        kConnect,               ///< 进入连接状态, 开始协商
        kHeartBeat,             ///< 进行一次心跳包
        kReadData,              ///< 读取数据走协议处理
        kSendSDK,               ///< 发送SDK数据
        kSendFile,              ///< 发送文件, 参数: 文件名, 文件类型(int)
        kReadFile,              ///< 读取文件
    };

    ///< 通知外部处理的指令
    enum eNotifyCmd {
        kNotofyConnect,         ///< 通知完成协商连接
        kSendData,              ///< 发送数据
        kReadyRead,             ///< 处理完毕的数据
        kReadyRawRead,          ///< 协议无法处理的数据, 将原始数据丢出
        kUploadFileProgress,    ///< 程序运行上传文件进度(文件名;;发送大小;;全部大小;;状态) 状态0正常, 1失败, 2完成
        kUploadFlowProgress,    ///< 上传流进度(发送大小;;全部大小;;状态)
        kSendFaildAnswer,       ///< 通知发送失败
        kSetFlowMonitor,        ///< 设置流量监控
    };

public:
    HSignal<void(const std::string &log)> DebugLog;

    ///< 通知指令, 要求外部关联着执行对应的指令
    HSignal<void(eNotifyCmd cmd, const HCatBuffer &data, const HCatAny &userData)> NotifyCmd;

public:
    explicit IProtocolBase(const EventCoreWeakPtr &core) : core_(core) {}
    virtual ~IProtocolBase() {}

    /**
     * @brief Dispose 协议处理
     * @param cmd 指令
     * @param data 数据
     * @param userData 自定义数据
     * @return true成功, false失败
     */
    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) = 0;

    virtual std::string Protocol() const = 0;

protected:
    struct delaySend
    {
        hint32 cmd;
        HCatBuffer data;
        HCatAny userData;

        delaySend(delaySend &&other) : cmd(std::move(other.cmd)), data(std::move(other.data)), userData(std::move(other.userData)) {}
        delaySend(hint32 cmd_, const HCatBuffer &data_, const HCatAny &user) : cmd(cmd_), data(data_), userData(user) {}

        delaySend &operator =(delaySend &&other) {
            this->cmd = std::move(other.cmd);
            this->data = std::move(other.data);
            this->userData = std::move(other.userData);
            return *this;
        }
    };


protected:
    EventCoreWeakPtr core_;
};


}


#endif // __IPROTOCOLBASE_H__
