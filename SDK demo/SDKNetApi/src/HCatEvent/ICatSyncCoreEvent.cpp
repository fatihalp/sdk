

#include <ICatSyncCoreEvent.h>
#include <HCatThread.h>


namespace cat
{


class ICatSyncCoreEventPrivate
{
public:
    ICatSyncCoreEventPrivate(const EventCoreWeakPtr &core)
        : core_(core)
        , update_(false)
        , exit_(false)
        , run_(false)
    {
        slot_ = ICatEventApi::NotifyUpdateMainThread.Bind(this, &ICatSyncCoreEventPrivate::NotifyUpdate);
    }
    ~ICatSyncCoreEventPrivate() {
        slot_.Clear();
        Quit();
    }

    void Quit() {
        {
            std::unique_lock<std::mutex> lock(lock_);
            exit_ = true;
        }
        wait_.notify_all();
        thread_.reset();
    }

private:
    void NotifyUpdate(const EventCoreWeakPtr &syncCore) {
        if (syncCore.lock() != core_) {
            return ;
        }

        {
            std::unique_lock<std::mutex> lock(lock_);
            update_ = true;
        }
        wait_.notify_all();
    }

public:
    EventCorePtrType core_;
    HSlot slot_;
    std::condition_variable wait_;
    std::mutex lock_;
    std::unique_ptr<HCatThread> thread_;
    bool update_;
    bool exit_;
    bool run_;
};


}


// ################################################

using namespace cat;


ICatSyncCoreEvent::ICatSyncCoreEvent(const EventCoreWeakPtr &core)
    : d_(new ICatSyncCoreEventPrivate(core))
{

}

ICatSyncCoreEvent::~ICatSyncCoreEvent()
{
    d_.reset();
}

bool ICatSyncCoreEvent::Exec()
{
    if (d_->exit_) {
        return false;
    }

    d_->run_ = true;
    ICatEventApi::SwitchMainThread(d_->core_);
    d_->update_ = ICatEventApi::IsNeedSyncEvent(d_->core_);
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(d_->lock_);
            d_->wait_.wait(lock, [this](){ return d_->update_ || d_->exit_; });
            d_->update_ = false;
        }

        if (d_->exit_ || !d_->core_) {
            break;
        }

        ICatEventApi::MainThreadSync(d_->core_);
    }

    d_->exit_ = false;
    d_->run_ = false;
    return true;
}

bool ICatSyncCoreEvent::Run()
{
    if (d_->thread_ || d_->run_) {
        return false;
    }

    d_->thread_.reset(new HCatThread());
    d_->thread_->Run(this, &ICatSyncCoreEvent::Exec);
    return true;
}

void ICatSyncCoreEvent::Quit()
{
    d_->Quit();
}

const EventCorePtrType &ICatSyncCoreEvent::GetEventCore() const
{
    return d_->core_;
}
