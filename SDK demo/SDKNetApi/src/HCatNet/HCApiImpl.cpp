

#include <HCApiImpl.h>
#include <ICatEventApi.h>


using namespace cat;


std::unique_ptr<HCApiImpl> HCApiImpl::instance_;

HCApiImpl *HCApiImpl::GetInstance()
{
    if (!instance_) {
        instance_.reset(new HCApiImpl());
    }

    return instance_.get();
}

void HCApiImpl::FreeInstance()
{
    instance_.reset();
}

HCApiImpl::~HCApiImpl()
{
    netList_.clear();
    syncCoreList_.clear();
    coreList_.clear();
}

ICatEventBase *HCApiImpl::CreateEventCore()
{
    std::unique_lock<std::mutex> lock(lock_);
    coreList_.emplace_back(IEventCore::CreateCoreEvnet());
    return coreList_.back().get();
}

void HCApiImpl::FreeEventCore(const ICatEventBase *core)
{
    if (!core) {
        return ;
    }

    FreeSyncEventNotifyBackCall(core);
    FreeSyncEventCore(core);
    std::unique_lock<std::mutex> lock(lock_);
    for (auto i = coreList_.begin(); i != coreList_.end(); ++i) {
        if (i->get() != core) {
            continue;
        }

        i = coreList_.erase(i);
        return ;
    }
}

HCApiImpl::syncCorePtr HCApiImpl::CreateSyncEventCore(const ICatEventBase *core)
{
    if (!core) {
        return syncCorePtr();
    }

    std::unique_lock<std::mutex> lock(lock_);
    auto corePtr = FindEventCore(core);
    for (const auto &i : syncCoreList_) {
        if (i->GetEventCore() != corePtr) {
            continue;
        }

        return i;
    }

    syncCoreList_.emplace_back(syncCorePtr(new HCatSyncCoreEvent(corePtr)));
    return syncCoreList_.back();
}

void HCApiImpl::FreeSyncEventCore(const ICatEventBase *core)
{
    if (!core) {
        return ;
    }

    std::unique_lock<std::mutex> lock(lock_);
    auto corePtr = FindEventCore(core);
    for (auto i = syncCoreList_.begin(); i != syncCoreList_.end(); ++i) {
        if (i->get()->GetEventCore() != corePtr) {
            continue;
        }

        i->get()->Quit();
        i = syncCoreList_.erase(i);
        return ;
    }
}

void HCApiImpl::SyncEventCore(const ICatEventBase *core)
{
    if (!core) {
        return ;
    }

    EventCorePtrType corePtr;
    {
        std::unique_lock<std::mutex> lock(lock_);
        corePtr = FindEventCore(core);
    }
    ICatEventApi::MainThreadSync(corePtr);
}

ISession *HCApiImpl::CreateNetSession(const ICatEventBase *core)
{
    if (!core) {
        return nullptr;
    }

    std::unique_lock<std::mutex> lock(lock_);
    auto corePtr = FindEventCore(core);
    if (!corePtr) {
        return nullptr;
    }

    netList_.emplace_back(netPtr(new ISession(corePtr)));
    return netList_.back().get();
}

void HCApiImpl::AppendNetSession(const netPtr &net)
{
    if (!net) {
        return ;
    }

    std::unique_lock<std::mutex> lock(lock_);
    netList_.push_back(net);
}

void HCApiImpl::FreeNetSession(const ISession *net)
{
    if (!net) {
        return ;
    }

    std::unique_lock<std::mutex> lock(lock_);
    for (auto i = netList_.begin(); i != netList_.end(); ++i) {
        if (i->get() != net) {
            continue;
        }

        ICatEventApi::Gc(net->GetEventCore(), std::move(*i));
        i = netList_.erase(i);
        return ;
    }
}

void HCApiImpl::AddSyncEventNotifyBackCall(const ICatEventBase *core, syncNotifyCallBackType &&backCall, void *userData)
{
    if (!core) {
        return ;
    }

    std::unique_lock<std::mutex> lock(lock_);
    syncNotifyMap_[core] = std::make_shared<HSyncNotifyCallBack>(std::move(backCall), userData);
}

void HCApiImpl::FreeSyncEventNotifyBackCall(const ICatEventBase *core)
{
    std::unique_lock<std::mutex> lock(lock_);
    syncNotifyMap_.erase(core);
}

HCApiImpl::HCApiImpl()
{
    slots_ += ICatEventApi::NotifyUpdateMainThread.Bind(this, &HCApiImpl::SyncEventNofity);
}


EventCorePtrType HCApiImpl::FindEventCore(const ICatEventBase *core)
{
    for (const auto &i : coreList_) {
        if (i.get() != core) {
            continue;
        }

        return i;
    }

    return EventCorePtrType();
}

void HCApiImpl::SyncEventNofity(const EventCoreWeakPtr &core)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return ;
    }

    HSyncNotifyCallBackPtr sync;
    {
        std::unique_lock<std::mutex> lock(lock_);
        if (syncNotifyMap_.find(corePtr.get()) == syncNotifyMap_.end()) {
            return ;
        }
        sync = syncNotifyMap_.at(corePtr.get());
    }

    if (!sync) {
        return ;
    }

    sync->backCall(corePtr.get(), sync->userData);
}
