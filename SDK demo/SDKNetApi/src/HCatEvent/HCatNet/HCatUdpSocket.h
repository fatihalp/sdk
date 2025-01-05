

#ifndef __HCATUDPSOCKET_H__
#define __HCATUDPSOCKET_H__


#include <HCatEventCore.h>
#include <HCatEventStruct.h>
#include <HCatSignal.h>
#include <ICatEventData.h>
#include <HCatBuffer.h>

#ifdef H_WIN
#include <WinSock2.h>
#endif


namespace cat
{


class HCATEVENT_EXPORT HCatUdpSocket : public HCatEventObject
{
public:
    ///< 数据接收完的时候将发送该信号
    HSignal<void(const std::string &ip, huint16 port, const HCatBuffer &data)> ReadData;

    ///< 成功连接将发送该信号
    HSignal<void()> Connected;

    ///< 断开连接将发送该信号
    HSignal<void()> Disconnected;

    ///< 连接错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

public:
    using IpType = huint32l;

public:
    explicit HCatUdpSocket(const EventCoreWeakPtr &core);
    virtual ~HCatUdpSocket();

    ///< 需要使用这个转换ip, 只有 kSendUnicast 才进行ip转换
    static IpType IP(const std::string &ip, eUdpSendType type = kSendUnicast);

    ///< 获取设备ip, 传入eth0网卡名
    static std::list<std::string> GetDeviceIP(const std::string &device);

    /**
     * @brief Connect 连接
     * @param sendIp 发送ip, 需要这个是因为多播的时候需要
     * @param sendPort 发送端口
     * @param sendType udp类型
     * @param listenIp 侦听网卡ip, 如果广播, 使用 kSendMulticast 代替 kSendBroadcast, 如果是指定ip则使用 kSendUnicast
     * @param listentPort 侦听端口
     * @param isBind 是否接收数据
     * @param device 是否指定网卡名, 设置了则优先网卡的ip, 失败后在选侦听ip, 不太完善
     * @return 成功返回true, 失败返回false
     */
    bool Connect(const std::string &sendIp, huint16 sendPort, eUdpSendType sendType = kSendBroadcast, const std::string &listenIp = "", huint16 listentPort = 0, bool isBind = true, const std::string &device = "");

    void Disconnect(bool signal = true);
    bool IsConnect() const;

    ///< 手动指定ip发送数据, 不依靠内部ip, 提供给需要指定ip发送的使用
    bool SendData(const std::string &ip, huint16 port, const HCatBuffer &data, eUdpSendType sendType = kSendUnicast);

    bool SendData(const HCatBuffer &data) { return SendData(sendIp_, port_, data, sendType_); }
private:
    void NotifyError(std::string *log = nullptr);

    void InitMember();
    void DestroyMember();
    void DisposeEvent(hint32 fd);
    void Close(bool updateSignal = true);

    void ParseSendData(const std::string &ip, huint16 port, const HCatBuffer &data, eUdpSendType sendType);
    bool ParseConnectSignal();
    void ParseError(const std::string &error);

private:
    EventCoreWeakPtr core_;
#ifdef H_WIN
    SOCKET socket_;
    WSADATA wsaData_;
#else
    hint32 socket_;
#endif
    std::string sendIp_;
    eUdpSendType sendType_;
    huint16 port_;
    bool threadExit_;
    bool threadRun_;
};


}

#endif // HCATUDPSOCKET_H
