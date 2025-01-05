

#include <Net/HTcpTlsNet.h>
#include <HCatNet/HCatTcpTlsSocket.h>
#include <HCatNet/HCatTcpSocket.h>
#include <ICatEventApi.h>


using namespace cat;


HTcpTlsNet::HTcpTlsNet(const EventCoreWeakPtr &core)
    : INetBase(core)
    , socket_(new HCatTcpTlsSocket(core))
{
    this->InitSignal();
}

HTcpTlsNet::HTcpTlsNet(const EventCoreWeakPtr &core, const std::shared_ptr<HCatTcpTlsSocket> &socket)
    : INetBase(core)
    , socket_(socket)
{
    this->InitSignal();
}

HTcpTlsNet::~HTcpTlsNet()
{
    this->set_.Clear();
    this->socket_->Disconnect(false);
}

void HTcpTlsNet::SetInfo(const HCatAny &info)
{
    if (info.IsType<TcpInfo>() == false) {
        return ;
    }

    info_ = info.Value<TcpInfo>();
}

bool HTcpTlsNet::Connect(bool async)
{
    std::string ip = info_.ip;
    auto ipList = HCatTcpSocket::UrlToIp(ip);
    if (ipList.empty() == false) {
        ip = std::move(ipList.front());
    }
    return socket_->Connect(ip, info_.port, async);
}

void HTcpTlsNet::Disconnect(bool signal)
{
    socket_->Disconnect(signal);
}

bool HTcpTlsNet::Isconnect() const
{
    return socket_->IsConnect();
}

bool HTcpTlsNet::Write(const HCatBuffer &data, const HCatAny &config)
{
    (void)config;
    return socket_->SendData(data);
}

void HTcpTlsNet::InitSignal()
{
    set_.Clear();

    set_ += socket_->Connected.Bind(Connected);
    set_ += socket_->Disconnected.Bind(Disconnected);
    set_ += socket_->ErrorInfo.Bind<const std::string &>(DebugLog);
    set_ += socket_->ReadData.Bind<const HCatBuffer &>(ReadyRead);
}
