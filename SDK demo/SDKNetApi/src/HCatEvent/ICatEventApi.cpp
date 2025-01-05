

#include <ICatEventApi.h>


using namespace cat;


HSignal<void(const EventCoreWeakPtr &)> ICatEventApi::NotifyUpdateMainThread;


void ICatEventApi::MainThreadSync(const EventCoreWeakPtr &core, const std::function<void ()> &eventCall, const HCatEventObjectInfoPtr &obj)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(kMainEvent);
    p->id = HCatEventId(obj);
    p->optEvent = kRegister;
    p->callback = eventCall;
    corePtr->Move(std::move(p));
}

HCatAny ICatEventApi::QueryMainEvent(const EventCoreWeakPtr &core, HCatUpdateDataEvent::eUpdateEvent query, kCatEventType event)
{
    auto corePtr = core.lock();
    if (!corePtr || event <= kMinEventLimit || event >= kMaxEventLimit) {
        return HCatAny();
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    HCatEventObjectInfoPtr info = std::make_shared<HCatEventObjectInfo>();
    p->optEvent = kUpdateData;
    p->id = HCatEventId(info);
    HCatUpdateDataEvent isMain(query);
    p->data = isMain;
    corePtr->Move(std::move(p));
    return *isMain.data;
}

bool ICatEventApi::IsMainThread(const EventCoreWeakPtr &core)
{
    HCatAny result = QueryMainEvent(core, HCatUpdateDataEvent::kIsCreateThread);
    if (result.IsType<bool>() == false) {
        return false;
    }

    return result.Cast<bool>();
}

void ICatEventApi::SwitchMainThread(const EventCoreWeakPtr &core)
{
    QueryMainEvent(core, HCatUpdateDataEvent::kUpdateMainThread);
}

bool ICatEventApi::IsNeedSyncEvent(const EventCoreWeakPtr &core)
{
    HCatAny result = QueryMainEvent(core, HCatUpdateDataEvent::kIsNeedSyncEvent);
    if (result.IsType<bool>() == false) {
        return false;
    }

    return result.Cast<bool>();
}

int ICatEventApi::GetIOReadWriteCount(const EventCoreWeakPtr &core)
{
    HCatAny result = QueryMainEvent(core, HCatUpdateDataEvent::kGetIOReadWriteCount, kIOReadWriteEvent);
    if (result.IsType<int>() == false) {
        return -1;
    }

    return result.Cast<int>();
}


void ICatEventApi::CreateEventSelectThread(const EventCoreWeakPtr &core, int event, int opt, std::function<void()> &&func, HCatAny &&data, HCatEventObjectPtr &&obj)
{
    auto corePtr = core.lock();
    if (!corePtr || obj.expired()) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(obj);
    p->callRun = HCatEventUnit::kMain;
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::forward<HCatAny>(data);
    corePtr->Move(std::move(p));
}


void ICatEventApi::CreateEventFunc(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func, HCatEventObjectPtr &&obj, HCatAny &&userData)
{
    auto corePtr = core.lock();
    if (!corePtr || obj.expired()) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(obj);
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::move(userData);
    corePtr->Move(std::move(p));
}

void ICatEventApi::CreateEventData(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func, HCatAny &&data)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::forward<HCatAny>(data);
    corePtr->Move(std::move(p));
}

void ICatEventApi::CreateEventData(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func, HCatAny &&data, HCatEventObjectPtr &&obj)
{
    auto corePtr = core.lock();
    if (!corePtr || obj.expired()) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(obj);
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::forward<HCatAny>(data);
    corePtr->Move(std::move(p));
}
