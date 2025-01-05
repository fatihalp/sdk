

#include <HCatSyncCoreEvent.h>
#include <ICatEventApi.h>
#include <HCatThread.h>
#include <IEventApi.h>
#include <ICatSyncCoreEvent.h>


using namespace cat;


HCatSyncCoreEvent::HCatSyncCoreEvent()
    : HCatSyncCoreEvent(HCatEventCore::CreateCoreEvent())
{

}

HCatSyncCoreEvent::HCatSyncCoreEvent(const EventCoreWeakPtr &core)
    : sync_(new ICatSyncCoreEvent(core))
{

}

HCatSyncCoreEvent::~HCatSyncCoreEvent()
{
    this->Quit();
    sync_.reset();
}

bool HCatSyncCoreEvent::Exec()
{
    return sync_->Exec();
}

bool HCatSyncCoreEvent::Run()
{
    return sync_->Run();
}

void HCatSyncCoreEvent::Quit()
{
    sync_->Quit();
}

const EventCorePtrType &HCatSyncCoreEvent::GetEventCore() const
{
    return sync_->GetEventCore();
}

