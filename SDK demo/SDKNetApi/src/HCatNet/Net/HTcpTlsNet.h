

#ifndef __HTCPTLSNET_H__
#define __HTCPTLSNET_H__


#include <Net/INetBase.h>
#include <HCatSignalSet.h>

#include <memory>


namespace cat
{


class HCatTcpTlsSocket;

class HCATNET_EXPORT HTcpTlsNet : public INetBase
{
public:
    explicit HTcpTlsNet(const EventCoreWeakPtr &core);
    explicit HTcpTlsNet(const EventCoreWeakPtr &core, const std::shared_ptr<HCatTcpTlsSocket> &socket);

    virtual ~HTcpTlsNet();
    virtual std::string Protocol() const override { return NetProtocol::HTcpTlsNet; }
    virtual HCatAny GetInfo() const override { return info_; }
    virtual void SetInfo(const HCatAny &info) override;
    virtual bool Connect(bool async = true) override;
    virtual void Disconnect(bool signal = true) override;
    virtual bool Isconnect() const override;
    virtual bool Write(const HCatBuffer &data, const HCatAny &config = HCatAny()) override;

private:
    void InitSignal();

private:
    std::shared_ptr<HCatTcpTlsSocket> socket_;
    TcpInfo info_;
    HCatSignalSet set_;
};


}


#endif // __HTCPTLSNET_H__
