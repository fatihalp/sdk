

#include <IPC/HCatDomainEvent.h>
#include <ICatEventApi.h>


using namespace cat;


HCatDomainEvent::HCatDomainEvent(const EventCoreWeakPtr &core)
    : core_(core)
{

}

bool HCatDomainEvent::Register(hint64 event, const std::string &path)
{
    std::unique_lock<std::mutex> lock(lock_);
    if (service_.find(event) != service_.end()) {
        return false;
    }

    HServerEventPtr service = std::make_shared<HServiceEvent>();
    service->unit.event = event;
    service->service = std::make_shared<HCatDomainService>(core_);
    if (!service->service) {
        return false;
    }

    service->unit.signalSet += service->service->NewConnect.Bind(this, &HCatDomainEvent::ParseNewClient);
    service->unit.signalSet += service->service->ErrorInfo.Bind<const std::string &>(ErrorInfo);
    if (service->service->Listen(path) == false) {
        return false;
    }

    service_[event] = service;

    return true;
}

void HCatDomainEvent::UnRegister(hint64 event)
{
    std::unique_lock<std::mutex> lock(lock_);
    service_.erase(event);
}

HCatDomainEvent::HClientEventPtr HCatDomainEvent::FindClient(HCatDomainSocket *clientPtr)
{
    HClientEventPtr result;
    std::unique_lock<std::mutex> lock(lock_);
    if (client_.find(clientPtr) == client_.end()) {
        return result;
    }

    return client_[clientPtr];
}

std::list<HCatDomainEvent::HClientEventPtr> HCatDomainEvent::FindClient(hint64 event)
{
    std::list<HClientEventPtr> result;
    std::unique_lock<std::mutex> lock(lock_);
    for (const auto &i : client_) {
        if (!i.second) {
            continue;
        }

        if (i.second->unit.event != event) {
            continue;
        }

        result.push_back(i.second);
    }

    return result;
}

void HCatDomainEvent::RemoveClient(HCatDomainSocket *clientPtr)
{
    auto p = FindClient(clientPtr);
    if (!p) {
        return ;
    }

    p->unit.signalSet.Clear();
    p->client->Disconnect(false);
    Disconnect(p->unit.event, p);
    auto *client = p->client.get();
    do {
        std::unique_lock<std::mutex> lock(lock_);
        client_.erase(client);
    } while (false);

    ///< 使用GC延迟释放是为了避免移除对象是自身发出的回调
    ICatEventApi::Gc(core_, std::move(p));
}

bool HCatDomainEvent::ExistClient(HCatDomainSocket *clientPtr)
{
    std::unique_lock<std::mutex> lock(lock_);
    return client_.find(clientPtr) != client_.end();
}

bool HCatDomainEvent::ExistEvent(hint64 event)
{
    std::unique_lock<std::mutex> lock(lock_);
    return service_.find(event) != service_.end();
}

void HCatDomainEvent::SendBroadCast(hint64 event, const HCatBuffer &data)
{
    const auto &client = FindClient(event);
    for (const auto &i : client) {
        if (i->client->SendData(data) == false) {
            RemoveClient(i->client.get());
        }
    }
}

void HCatDomainEvent::ParseNewClient(std::shared_ptr<HCatDomainSocket> client, HCatDomainService *service)
{
    if (!client) {
        return ;
    }

    auto servicePtr = FindService(service);
    if (!servicePtr) {
        return ;
    }

    hint64 event = servicePtr->unit.event;
    HClientEventPtr p = std::make_shared<HClientEvent>();
    p->unit.event = servicePtr->unit.event;
    p->client = client;
    p->unit.signalSet += client->Disconnected.Bind(this, &HCatDomainEvent::ParseClientDisconnect);
    p->unit.signalSet += client->ReadData.Bind(this, &HCatDomainEvent::ParseClientReadData);
    p->unit.signalSet += client->ErrorInfo.Bind<const std::string &>(ErrorInfo);

    do {
        std::unique_lock<std::mutex> lock(lock_);
        client_[client.get()] = p;
    } while (false);

    ICatEventApi::ForwardObjFunc(core_, true, this, [this, event, p]() {
        NewClient(event, p);
    });
}

void HCatDomainEvent::ParseClientReadData(const HCatBuffer &data, HCatDomainSocket *client)
{
    auto p = FindClient(client);
    if (!p) {
        return ;
    }

    ICatEventApi::ForwardObjFunc(core_, true, this, [this, data, p]() {
        ReadyRead(p->unit.event, data, p);
    });
}

void HCatDomainEvent::ParseClientDisconnect(HCatDomainSocket *client)
{
    RemoveClient(client);
}

HCatDomainEvent::HServerEventPtr HCatDomainEvent::FindService(HCatDomainService *servicePtr)
{
    std::unique_lock<std::mutex> lock(lock_);
    for (const auto &i : service_) {
        if (!i.second) {
            continue;
        }

        if (i.second->service.get() == servicePtr) {
            return i.second;
        }
    }
    return HServerEventPtr();
}
