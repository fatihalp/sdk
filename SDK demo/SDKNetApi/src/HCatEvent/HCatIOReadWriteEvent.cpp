


#include <HCatIOReadWriteEvent.h>


using namespace cat;


HCatIOReadWriteEvent::HCatIOReadWriteEvent(const EventCoreWeakPtr &core)
    : core_(core)
    , thread_(new HCatThread)
    , taskNum_(0)
    , exit_(false)
{

}

HCatIOReadWriteEvent::~HCatIOReadWriteEvent()
{
    this->exit_ = true;
    thread_.reset();
}

void HCatIOReadWriteEvent::Move(H_EventPtr &&event)
{
    if (core_.expired()) {
        return ;
    }

    if (!event || event->id.IsDeathEvent(HCatEventObjectInfo::kDeathIO)) {
        return ;
    }

    switch (event->optEvent) {
    case kIOWriteCall: {
        PushWrite(std::forward<H_EventPtr>(event));
    } break;
    case kIOReadCall:
    case kRegister: {
        PushRead(std::forward<H_EventPtr>(event));
    } break;
    case kUpdateData: {
        ParseQueryEvent(std::forward<H_EventPtr>(event));
    } break;
    default:
        break;
    }

    return ;
}

void HCatIOReadWriteEvent::NotifyRun()
{
    if (thread_->IsRun() && thread_->Size() > 1) {
        return ;
    }

    thread_->Run(this, &HCatIOReadWriteEvent::Loop);
}

void HCatIOReadWriteEvent::PushWrite(H_EventPtr &&task)
{
    std::unique_lock<std::mutex> lock(lock_);
    writeList_.emplace_back(std::forward<H_EventPtr>(task));
    this->NotifyRun();
}

void HCatIOReadWriteEvent::PushRead(H_EventPtr &&task)
{
    std::unique_lock<std::mutex> lock(lock_);
    readList_.emplace_back(std::forward<H_EventPtr>(task));
    this->NotifyRun();
}

void HCatIOReadWriteEvent::ParseQueryEvent(H_EventPtr &&event)
{
    if (event->data.IsType<HCatUpdateDataEvent>() == false) {
        return ;
    }

    HCatUpdateDataEvent &query = event->data.Cast<HCatUpdateDataEvent>();
    switch (query.query) {
    case HCatUpdateDataEvent::kGetIOReadWriteCount: {
        *query.data = static_cast<int>(taskNum_ + writeList_.size() + readList_.size());
    } break;
    default:
        break;
    }
    return ;
}

void HCatIOReadWriteEvent::Loop()
{
    for (;;) {
        if (exit_ || core_.expired()) {
            return ;
        }

        // 直接和数据进行交换, 确保锁停留的时间最短
        std::list<H_EventPtr> opt;
        {
            // 读事件优先处理
            std::unique_lock<std::mutex> lock(lock_);
            if (readList_.empty() == false) {
                opt.splice(opt.end(), std::move(readList_));
            }
            taskNum_ = opt.size();
        }

        {
            // 写事件降权
            std::unique_lock<std::mutex> lock(lock_);
            if (writeList_.empty() == false) {
                opt.splice(opt.end(), std::move(writeList_));
            }
            taskNum_ = opt.size();
        }


        if (opt.empty()) {
            return ;
        }

        this->ParseTask(std::move(opt));
    }
}

void HCatIOReadWriteEvent::ParseTask(std::list<H_EventPtr> &&task)
{
    while (task.empty() == false) {
        taskNum_ = task.size();
        H_EventPtr i(std::move(task.front()));
        task.pop_front();

        bool writeEvent = i->optEvent == kIOWriteCall;
        RunTask(std::move(i));

        // 执行写事件的时候去调度一次读事件
        if (writeEvent) {
            RunOnceRead();
        }
    }
}

void HCatIOReadWriteEvent::RunOnceRead()
{
    H_EventPtr task;
    {
        std::unique_lock<std::mutex> lock(lock_);
        if (readList_.empty()) {
            return ;
        }
        task = std::move(readList_.front());
        readList_.pop_front();
    }
    RunTask(std::move(task));
}

void HCatIOReadWriteEvent::RunTask(H_EventPtr &&task)
{
    if (!task) {
        return ;
    }

    if (task->id.IsDeathEvent(HCatEventObjectInfo::kDeathIO)) {
        return ;
    }

    if (task->callback) {
        task->callback();
    }

    // IO写事件, 直接销毁提高速度
    if (task->optEvent == kIOWriteCall) {
        return ;
    }

    task->optEvent = kCallEnd;

    auto core = core_.lock();
    if (!core) {
        return ;
    }
    core->Move(std::move(task));
}
