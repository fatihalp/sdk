

#ifndef __HCATKCPSOCKET_H__
#define __HCATKCPSOCKET_H__


#include <HCatEventCore.h>
#include <HCatEventStruct.h>
#include <HCatSignal.h>
#include <ICatEventData.h>
#include <HCatBuffer.h>


namespace cat
{


class HCatKcpUnitPrivate;
class HCatKcpSocketPrivate;

///< kcp单元, 发送数据和接收数据需要这个
class HCATEVENT_EXPORT HCatKcpUnit
{
public:
    ///< 数据接收完的时候将发送该信号
    HSignal<void(const std::string &ip, huint16 port, const HCatBuffer &data, HCatKcpUnit *unit)> ReadData;

public:
    HCatKcpUnit(std::weak_ptr<HCatKcpSocketPrivate> socket, huint32 conv, const std::string &ip, huint16 port);

    bool SendData(const HCatBuffer &data);
private:
    void CheckReadData();

private:
    friend class HCatKcpSocket;
    friend class HCatKcpUnitPrivate;
    std::unique_ptr<HCatKcpUnitPrivate> d_;
};
using HCatKcpUnitPtr = std::shared_ptr<HCatKcpUnit>;
using HCatKcpUnitWeakPtr = std::weak_ptr<HCatKcpUnit>;



class HCATEVENT_EXPORT HCatKcpSocket : public HCatEventObject
{
public:
    ///< 成功连接将发送该信号
    HSignal<void()> Connected;

    ///< 断开连接将发送该信号
    HSignal<void()> Disconnected;

    ///< 接受到的错误数据包
    HSignal<void(const std::string &ip, huint16 port, const HCatBuffer &data)> ErrorReadData;

    ///< 接收到本地不存在的kcp, 生成新的kcp单元, 信号发射后需要绑定关联, 否则数据可能丢失
    HSignal<void(const HCatKcpUnitPtr &unit)> NewKcpUnit;

public:
    explicit HCatKcpSocket(const EventCoreWeakPtr &core);
    ~HCatKcpSocket();

    /**
     * @brief Listen 开启侦听udp
     * @param listenPort 侦听端口
     * @param listenIp 侦听网卡, 可默认空
     * @param device 侦听网卡名, 可默认空
     * @return 成功返回true, 失败返回false
     */
    bool Listen(huint16 listenPort, const std::string &listenIp = "", const std::string &device = "");
    void Disconnect(bool signal = true);

    /**
     * @brief CreateKcpUnit 创建kcp单元
     * @param ip 通讯对端ip
     * @param port 对端端口
     * @param conv 通讯conv, 需要相同才能通讯, 默认生成随机数
     * @return 返回创建的kcp通讯单元, 如果已存在则不会创建, 返回已经存在的
     */
    HCatKcpUnitPtr CreateKcpUnit(const std::string &ip, huint16 port, huint32 conv = std::numeric_limits<huint32>::max());
    HCatKcpUnitPtr FindKcpUnit(const std::string &ip, huint16 port, huint32 conv);

    /**
     * 极速模式: SetKcpUnit(unit, 1, 10, 2, 1);
     * 普通模式: SetKcpUnit(unit, 0, 40, 0, 0);
     * nc: 是否关闭流控，默认是0代表不关闭，1代表关闭。
     * resend: 快速重传模式，默认0关闭，可以设置2（2次ACK跨越将会直接重传）
     * interval: 协议内部工作的 interval，单位毫秒，比如 10ms或者 20ms
     * nodelay: 是否启用 nodelay模式，0不启用；1启用。
     */
    void SetKcpUnit(HCatKcpUnitPtr unit, int nodelay, int interval, int resend, int nc);

    /**
     * @brief SendData 发送数据
     * @param unit kcp单元
     * @param data 数据
     * @return 发送失败返回false
     */
    bool SendData(const HCatKcpUnit *unit, const HCatBuffer &data);
    bool SendData(HCatKcpUnitPtr unit, const HCatBuffer &data) { return SendData(unit.get(), data); }

private:
    void ParseUdpData(const std::string &ip, huint16 port, const HCatBuffer &data);
    bool ParseSendData(const HCatKcpUnit * unit, const HCatBuffer &data);
    void UpdateKcp();

private:
    friend class HCatKcpSocketPrivate;
    std::shared_ptr<HCatKcpSocketPrivate> d_;
};


}


#endif // __HCATKCPSOCKET_H__
