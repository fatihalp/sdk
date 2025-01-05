

#ifndef __HCATTCPTLSSOCKET_H__
#define __HCATTCPTLSSOCKET_H__


#include <HCatEventCore.h>
#include <HCatEventStruct.h>
#include <HCatThread.h>
#include <HCatSignal.h>
#include <HCatBuffer.h>
#include <HCatOpenSSL.h>

#ifdef H_WIN
#include <WinSock2.h>
#endif


namespace cat
{


class HCATEVENT_EXPORT HCatTcpTlsSocket : public HCatEventObject
{
public:
    ///< 数据接收完的时候将发送该信号
    HSignal<void(const HCatBuffer &data)> ReadData;

    ///< 成功连接将发送该信号
    HSignal<void()> Connected;

    ///< 断开连接将发送该信号
    HSignal<void()> Disconnected;

    ///< 连接错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

public:
    explicit HCatTcpTlsSocket(const EventCoreWeakPtr &core);
    virtual ~HCatTcpTlsSocket();

    bool Connect(const std::string &ip, huint16 port, bool threadRun = true, const std::string &bindIp = std::string(), huint16 bindPort = 0);
    void Disconnect(bool signal = true);
    bool IsConnect() const;

    bool SendData(const HCatBuffer &data);

    std::string GetIp() const { return ip_; }
    huint16 GetPort() const { return port_; }

    bool GetInfo(std::string &ip, huint16 &port);

private:
    void InitMember();
    void DestroyMember();

    void Close(bool updateSignal = true, bool resetId = true);
    void DisposeEvent(hint32 fd);
    void ParseSendData(const HCatBuffer &data);
    void ParseConnect(const std::string &ip, huint16 port, const HCatAny &data);
    bool ParseConnectSignal();
    void ParseError(const std::string &error);
    void NotifyError(std::string *log = nullptr);

private:
    EventCoreWeakPtr core_;
    HCatThread thread_;

    HCatOpenSSL::CtxPtr ctx_;
    HCatOpenSSL::SSLPtr ssl_;
#ifdef H_WIN
    SOCKET socket_;
    WSADATA wsaData_;
#else
    hint32 socket_;
#endif

    std::string ip_;
    hint16 port_;
    bool threadExit_;
    bool threadRun_;
};



}


#endif // HCATTCPTLSSOCKET_H
