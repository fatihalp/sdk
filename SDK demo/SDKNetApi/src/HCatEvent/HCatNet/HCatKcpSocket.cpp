

#include <HCatNet/HCatKcpSocket.h>
#include <HCatNet/HCatUdpSocket.h>
#include <HCatTimer.h>
#include <HCatSignalSet.h>
#include <ikcp.h>
#include <chrono>
#include <list>
#include <map>
#include <random>


#define KCP_UPDATE_TIME     (100)
#define BUFF_MAX            (1024 * 20)

namespace cat
{


class HCatKcpSocketPrivate
{
public:
    HCatKcpSocketPrivate(const EventCoreWeakPtr &eventCore, HCatKcpSocket *socket)
        : core(eventCore)
        , p(socket)
        , udp(core)
    {}
    ~HCatKcpSocketPrivate() { signalSet.Clear(); updateKcp.Stop(); }

    EventCoreWeakPtr core;      ///< 事件核心
    HCatKcpSocket *p;
    HCatUdpSocket udp;
    HCatTimer updateKcp;        ///< 更新kcp定时器
    HCatSignalSet signalSet;    ///< 绑定信号集
    std::map<IUINT32, std::list<HCatKcpUnitWeakPtr>> unitList;
    std::mutex unitListLock;

    bool SendData(const std::string &ip, huint16 port, const HCatBuffer &data) { return udp.SendData(ip, port, data, kSendUnicast); }
};


class HCatKcpUnitPrivate
{
public:
    HCatKcpUnitPrivate(HCatKcpUnit *unit, std::weak_ptr<HCatKcpSocketPrivate> socket, huint32 conv, const std::string &ip_, huint16 port_)
        : socketIn_(socket)
        , kcp(nullptr)
        , ip(ip_)
        , port(port_)
    {
        kcp = ikcp_create(conv, unit);
        kcp->output = &HCatKcpUnitPrivate::kcpSend;
    }

    ~HCatKcpUnitPrivate() {
        if (kcp) {
            ikcp_release(kcp);
            kcp = nullptr;
        }
    }

    static int kcpSend(const char *buf, int len, ikcpcb *kcp, void *user) {
        assert(user);
        HCatKcpUnit *This = reinterpret_cast<HCatKcpUnit *>(user);
        if (This->d_->kcp != kcp) {
            return -1;
        }

        auto p = This->d_->socketIn_.lock();
        if (!p) {
            return -1;
        }

        return p->SendData(This->d_->ip, This->d_->port, HCatBuffer(buf, len)) ? len : -1;
    }

    std::weak_ptr<HCatKcpSocketPrivate> socketIn_;
    ikcpcb *kcp;
    std::string ip;
    huint16 port;
};



}


using namespace cat;


HCatKcpUnit::HCatKcpUnit(std::weak_ptr<HCatKcpSocketPrivate> socket, huint32 conv, const std::string &ip, huint16 port)
    : d_(new HCatKcpUnitPrivate(this, socket, conv, ip, port))
{

}

bool HCatKcpUnit::SendData(const HCatBuffer &data)
{
    auto p = d_->socketIn_.lock();
    if (!p) {
        return false;
    }

    return p->p->SendData(this, data);
}

void HCatKcpUnit::CheckReadData()
{
    if (!d_->kcp) {
        return ;
    }

    char data[BUFF_MAX];
    for (;;) {
        int len = ikcp_recv(d_->kcp, data, BUFF_MAX);
        if (len <= 0) {
            return ;
        }

        ReadData(d_->ip, d_->port, HCatBuffer(data, len), this);
    }
}


///< ##############################################


HCatKcpSocket::HCatKcpSocket(const EventCoreWeakPtr &core)
    : d_(new HCatKcpSocketPrivate(core, this))
{
    d_->signalSet += d_->updateKcp.Timeout.Bind(this, &HCatKcpSocket::UpdateKcp);
    d_->signalSet += d_->udp.Connected.Bind(Connected);
    d_->signalSet += d_->udp.Disconnected.Bind(Disconnected);
    d_->signalSet += d_->udp.ReadData.Bind(this, &HCatKcpSocket::ParseUdpData);
}

HCatKcpSocket::~HCatKcpSocket()
{
    d_->signalSet.Clear();
    d_.reset();
}

bool HCatKcpSocket::Listen(huint16 listenPort, const std::string &listenIp, const std::string &device)
{
    bool status = d_->udp.Connect("", 0, kSendUnicast, listenIp, listenPort, true, device);
    if (status) {
        d_->updateKcp.Start(d_->core, KCP_UPDATE_TIME);
    }
    return status;
}

void HCatKcpSocket::Disconnect(bool signal)
{
    d_->udp.Disconnect(signal);
    d_->unitList.clear();
    d_->updateKcp.Stop();
}

HCatKcpUnitPtr HCatKcpSocket::CreateKcpUnit(const std::string &ip, huint16 port, huint32 conv)
{
    if (conv == std::numeric_limits<huint32>::max()) {
        static std::default_random_engine e(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        static std::uniform_int_distribution<huint32> u(0, std::numeric_limits<huint32>::max() - 1);
        conv = u(e);
    }

    auto p = FindKcpUnit(ip, port, conv);
    if (p) {
        return p;
    }

    p = std::make_shared<HCatKcpUnit>(d_, conv, ip, port);
    SetKcpUnit(p, 1, 100, 2, 1);

    std::unique_lock<std::mutex> lock(d_->unitListLock);
    d_->unitList[p->d_->kcp->conv].push_back(p);
    return p;
}

HCatKcpUnitPtr HCatKcpSocket::FindKcpUnit(const std::string &ip, huint16 port, huint32 conv)
{
    HCatKcpUnitPtr ret;
    std::unique_lock<std::mutex> lock(d_->unitListLock);
    auto index = d_->unitList.find(conv);
    if (d_->unitList.end() == index) {
        return ret;
    }

    for (const auto &i : index->second) {
        auto p = i.lock();
        if (!p) {
            continue;
        }

        if (p->d_->ip != ip || p->d_->port != port) {
            continue;
        }

        return p;
    }

    return ret;
}

void HCatKcpSocket::SetKcpUnit(HCatKcpUnitPtr unit, int nodelay, int interval, int resend, int nc)
{
    if (!unit) {
        return ;
    }

    ikcp_nodelay(unit->d_->kcp, nodelay, interval, resend, nc);
}


bool HCatKcpSocket::SendData(const HCatKcpUnit *unit, const HCatBuffer &data)
{
    // 检查是否是这个创建的
    if (!unit || unit->d_->socketIn_.lock() != d_) {
        return false;
    }

    if (!unit->d_->kcp) {
        return false;
    }

    if (d_->udp.IsConnect() == false) {
        return false;
    }

    // 是这个事件线程那就直接发送
    if (ICatEventApi::IsMainThread(d_->core)) {
        return ParseSendData(unit, data);
    }

    ICatEventApi::Forward(d_->core, true, this, &HCatKcpSocket::ParseSendData, unit, data);
    return d_->udp.IsConnect();
}


void HCatKcpSocket::ParseUdpData(const std::string &ip, huint16 port, const HCatBuffer &data)
{
    if (data.Size() < sizeof(IUINT32)) {
        ErrorReadData(ip, port, data);
        return ;
    }

    IUINT32 conv = ikcp_getconv(data.ConstData());
    bool add = true;
    do {
        std::unique_lock<std::mutex> lock(d_->unitListLock);
        auto index = d_->unitList.find(conv);
        if (index == d_->unitList.end()) {
            d_->unitList[conv];
            index = d_->unitList.find(conv);
        }

        for (auto i = index->second.begin(); i != index->second.end();) {
            auto p = i->lock();
            if (!p) {
                i = index->second.erase(i);
                continue;
            }

            if (p->d_->port != port || p->d_->ip != ip) {
                ++i;
                continue;
            }

            add = false;
            ikcp_input(p->d_->kcp, data.ConstData(), data.Size());
            p->CheckReadData();
            break;
        }
    } while(false);

    if (add) {
        auto kcp = CreateKcpUnit(ip, port, conv);
        NewKcpUnit(kcp);
        ikcp_input(kcp->d_->kcp, data.ConstData(), data.Size());
        kcp->CheckReadData();
    }
}

bool HCatKcpSocket::ParseSendData(const HCatKcpUnit *unit, const HCatBuffer &data)
{
    if (!unit) {
        return false;
    }

    return ikcp_send(unit->d_->kcp, data.ConstData(), data.Size()) == 0;
}


void HCatKcpSocket::UpdateKcp()
{
    std::unique_lock<std::mutex> lock(d_->unitListLock);
    for (auto i = d_->unitList.begin(); i != d_->unitList.end(); ) {
        for (auto j = i->second.begin(); j != i->second.end();) {
            auto p = j->lock();
            if (!p) {
                j = i->second.erase(j);
                continue;
            }

            ikcp_update(p->d_->kcp, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
            p->CheckReadData();
            ++j;
        }

        if (i->second.empty()) {
            i = d_->unitList.erase(i);
            continue;
        }
        ++i;
    }
}
