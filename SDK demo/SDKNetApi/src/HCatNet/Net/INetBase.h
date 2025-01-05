

#ifndef __INETBASE_H__
#define __INETBASE_H__


#include <HCatAny.h>
#include <HCatNetInfo.h>
#include <HCatSignal.h>
#include <ICatEventApi.h>
#include <ICatEventData.h>
#include <HCatEventStruct.h>
#include <HCatBuffer.h>


namespace cat
{


class HCATNET_EXPORT INetBase
{
public:
    HSignal<void(const std::string &log)> DebugLog;
    HSignal<void()> Connected;
    HSignal<void()> Disconnected;
    HSignal<void(const HCatBuffer &data)> ReadyRead;
    HSignal<void(const std::string &ip, huint16 port, const HCatBuffer &data)> ReadyReadUdp;
    HSignal<void(const std::shared_ptr<INetBase> &)> NetConnect;

public:
    struct TcpInfo
    {
        std::string ip;
        huint16 port;
        TcpInfo() : port(0) {}
        TcpInfo(const std::string &ip_, huint16 port_) : ip(ip_), port(port_) {}
    };

    struct UdpInfo
    {
        std::string sendIp;
        std::string ip;
        huint16 sendPort;
        huint16 port;
        eUdpSendType sendType;      ///< 发送udp是单播, 组播还是广播, 默认广播

        UdpInfo() : sendPort(0), port(0), sendType(kSendBroadcast) {}
        UdpInfo(const std::string &listenIp, huint16 listenPort, const std::string &sendIp_, huint16 sendPort_, eUdpSendType sendType_ = kSendBroadcast) : sendIp(sendIp_), ip(listenIp), sendPort(sendPort_), port(listenPort), sendType(sendType_) {}
    };

    struct SerialInfo
    {
        std::string com;
        eBaudRate baudRate;
        eDataBits data;
        eParity parity;
        eStopBits stopBit;

        SerialInfo() : baudRate(kBaud9600), data(kData8), parity(kNoParity), stopBit(kOneStop) {}
        SerialInfo(const std::string &com_, eBaudRate baud, eDataBits dataBit, eParity par, eStopBits stop) : com(com_), baudRate(baud), data(dataBit), parity(par), stopBit(stop) {}
    };

public:
    explicit INetBase(const EventCoreWeakPtr &core) : core_(core) {}
    virtual ~INetBase() {}

    virtual std::string Protocol() const = 0;
    virtual HCatAny GetInfo() const = 0;
    virtual void SetInfo(const HCatAny &info) = 0;
    virtual bool Connect(bool async = true) = 0;
    virtual void Disconnect(bool signal = true) = 0;
    virtual bool Isconnect() const = 0;
    virtual bool Write(const HCatBuffer &data, const HCatAny &config = HCatAny()) = 0;
    virtual void AddFlowMonitor(const std::weak_ptr<HFlowCount> &) {}

protected:
    EventCoreWeakPtr core_;
};


}


#endif // __INETBASE_H__
