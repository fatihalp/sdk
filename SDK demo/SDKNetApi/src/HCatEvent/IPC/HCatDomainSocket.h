

#ifndef __HCATDOMAINSOCKET_H__
#define __HCATDOMAINSOCKET_H__


#include <HCatEventCore.h>
#include <HCatEventStruct.h>
#include <HCatSignal.h>
#include <HCatBuffer.h>


namespace cat
{


class HCATEVENT_EXPORT HCatDomainSocket : public HCatEventObject
{
public:
    ///< 数据接收完的时候将发送该信号
    HSignal<void(const HCatBuffer &data, HCatDomainSocket *)> ReadData;

    ///< 断开连接将发送该信号
    HSignal<void(HCatDomainSocket *)> Disconnected;

    ///< 连接错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

public:
    explicit HCatDomainSocket(const EventCoreWeakPtr &core);
    virtual ~HCatDomainSocket();

protected:
    friend class HCatDomainService;
    ///< 用于服务端产生的连接(服务端产生的连接是必须调用这个才会进行数据处理)
    explicit HCatDomainSocket(const EventCoreWeakPtr &core, int socket);
    bool Connect();

public:
    ///< 用于客户端的
    bool Conncet(const std::string &path);
    void Disconnect(bool signal = true);
    bool IsConnect() const;

    bool SendData(const HCatBuffer &data);

private:
    void Close(bool updateSignal = true, bool resetId = true);
    void DisposeEvent(hint32 fd);
    void ParseSendData(const HCatBuffer &data);
    void ParseError(const std::string &error);
    void NotifyError(std::string *log = nullptr);

private:
    EventCoreWeakPtr core_;
    hint32 socket_;
    bool threadExit_;
    bool threadRun_;
};


}


#endif // __HCATDOMAINSOCKET_H__
