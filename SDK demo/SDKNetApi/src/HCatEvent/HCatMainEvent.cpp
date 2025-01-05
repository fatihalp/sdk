﻿

#include <HCatMainEvent.h>
#include <ICatEventApi.h>


using namespace cat;


HCatMainEvent::HCatMainEvent(const EventCoreWeakPtr &core)
    : core_(core)
    , threadId_(std::this_thread::get_id())
    , run_(true)
{

}

HCatMainEvent::~HCatMainEvent()
{
    run_ = false;
}

void HCatMainEvent::Move(H_EventPtr &&event)
{
    if (core_.expired()) {
        return ;
    }

    const hint32 opt = event->optEvent;
    switch (opt) {
    case kRegister: {
        this->ParseTask(std::forward<H_EventPtr>(event));
    } break;
    case kMainRegister: {
        event->optEvent = kRegister;
        this->Push(std::forward<H_EventPtr>(event), true);
    } break;
    case kCall: {
        this->Push(std::forward<H_EventPtr>(event));
    } break;
    case kUpdateData: {
        this->ParseQueryEvent(std::forward<H_EventPtr>(event));
    } break;
    default:
        break;
    }

    return ;
}

void HCatMainEvent::Push(H_EventPtr &&event, bool forward)
{
    bool notify = false;
    if (forward) {
        std::unique_lock<std::mutex> lock(lock_);
        notify = taskList_.empty() && forwardList_.empty();
        forwardList_.emplace_back(std::forward<H_EventPtr>(event));
    } else {
        std::unique_lock<std::mutex> lock(lock_);
        notify = taskList_.empty();
        taskList_.emplace_back(std::forward<H_EventPtr>(event));
    }

    if (notify && core_.expired() == false) {
        ICatEventApi::NotifyUpdateMainThread(core_);
    }
}


void HCatMainEvent::ParseTask(H_EventPtr &&event)
{
    if (std::this_thread::get_id() != threadId_) {
        if (event->id.IsInvalid() == false && event->callback) {
            event->callback();
        }

        return ;
    }

    std::list<H_EventPtr> task;
    std::list<H_EventPtr> forwardList;
    {
        std::unique_lock<std::mutex> lock(lock_);
        forwardList.swap(forwardList_);
        task.swap(taskList_);
    }

    ///< 由主事件转发任务到各个事件进行
    while (forwardList.empty() == false) {
        H_EventPtr i(std::move(forwardList.front()));
        forwardList.pop_front();

        if (run_ == false) {
            return ;
        }

        if (!i) {
            continue;
        }

        auto core = core_.lock();
        if (!core) {
            return ;
        }
        core->Move(std::move(i));
    }

    ///< 进行同步任务
    while (task.empty() == false) {
        H_EventPtr i(std::move(task.front()));
        task.pop_front();
        if (run_ == false) {
            return ;
        }

        if (i->id.IsInvalid()) {
            continue;
        }

        if (i->callback) {
            i->callback();
        }
        i->optEvent = kCallEnd;

        auto core = core_.lock();
        if (!core) {
            return ;
        }
        core->Move(std::move(i));
    }

    bool notify = false;
    {
        std::unique_lock<std::mutex> lock(lock_);
        notify = taskList_.empty() == false || forwardList.empty() == false;
    }

    if (notify && core_.expired() == false) {
        ICatEventApi::NotifyUpdateMainThread(core_);
    }

    if (event->id.IsInvalid() == false && event->callback) {
        event->callback();
    }

    // 去执行GC回收
    auto core = core_.lock();
    if (!core) {
        return ;
    }
    core->Move(std::make_shared<HCatEventUnit>(kGcEvent, kCall, HCatEventUnit::kCall));
}

void HCatMainEvent::ParseQueryEvent(H_EventPtr &&event)
{
    if (event->data.IsType<HCatUpdateDataEvent>() == false) {
        return ;
    }

    HCatUpdateDataEvent &query = event->data.Cast<HCatUpdateDataEvent>();

    switch (query.query) {
    case HCatUpdateDataEvent::kIsCreateThread: {
        *query.data = bool(threadId_ == std::this_thread::get_id());
    } break;
    case HCatUpdateDataEvent::kIsNeedSyncEvent: {
        std::unique_lock<std::mutex> lock(lock_);
        *query.data = bool(taskList_.empty() == false);
    } break;
    case HCatUpdateDataEvent::kUpdateMainThread: {
        threadId_ = std::this_thread::get_id();
    } break;
    default:
        break;
    }
    return ;
}
