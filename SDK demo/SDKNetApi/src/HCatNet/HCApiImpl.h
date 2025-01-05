

#ifndef __HCAPIIMPL_H__
#define __HCAPIIMPL_H__


#include <IEventApi.h>
#include <HCatSyncCoreEvent.h>
#include <map>


namespace cat
{


class HCApiImpl
{
public:
    using netPtr = std::shared_ptr<ISession>;
    using syncCorePtr = std::shared_ptr<HCatSyncCoreEvent>;
    using syncNotifyCallBackType = std::function<void(const ICatEventBase *core, void *userData)>;

public:
    static HCApiImpl *GetInstance();
    static void FreeInstance();
    ~HCApiImpl();

    ICatEventBase *CreateEventCore();
    void FreeEventCore(const ICatEventBase *core);

    syncCorePtr CreateSyncEventCore(const ICatEventBase *core);
    void FreeSyncEventCore(const ICatEventBase *core);
    void SyncEventCore(const ICatEventBase *core);

    ISession *CreateNetSession(const ICatEventBase *core);
    void AppendNetSession(const netPtr &net);
    void FreeNetSession(const ISession *net);

    void AddSyncEventNotifyBackCall(const ICatEventBase *core, syncNotifyCallBackType &&backCall, void *userData);
    void FreeSyncEventNotifyBackCall(const ICatEventBase *core);

    std::size_t CoreSize() const { return coreList_.size(); }
private:
    HCApiImpl();
    EventCorePtrType FindEventCore(const ICatEventBase *core);

    void SyncEventNofity(const EventCoreWeakPtr &core);

private:
    struct HSyncNotifyCallBack
    {
        syncNotifyCallBackType backCall;
        void *userData;
        HSyncNotifyCallBack() : userData(nullptr) {}
        explicit HSyncNotifyCallBack(syncNotifyCallBackType && call, void *user) : backCall(std::move(call)), userData(user) {}
    };
    using HSyncNotifyCallBackPtr = std::shared_ptr<HSyncNotifyCallBack>;

private:
    static std::unique_ptr<HCApiImpl> instance_;

    std::mutex lock_;
    std::list<EventCorePtrType> coreList_;  ///< 事件核心列表
    std::list<syncCorePtr> syncCoreList_;   ///< 自动同步事件核心任务列表
    std::list<netPtr> netList_;             ///< 会话列表
    std::map<const ICatEventBase *, HSyncNotifyCallBackPtr> syncNotifyMap_;   ///< 手动同步事件核心的回调通知函数表

    HCatSignalSet slots_;
};


}


#define CApiImpl() \
    cat::HCApiImpl::GetInstance()


#endif // __HCAPIIMPL_H__
