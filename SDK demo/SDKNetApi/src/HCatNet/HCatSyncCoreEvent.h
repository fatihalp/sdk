

#ifndef __HCATSYNCCOREEVENT_H__
#define __HCATSYNCCOREEVENT_H__


#include <IEventApi.h>


namespace cat
{


class ICatSyncCoreEvent;

/**
 * @brief The HCatSyncCoreEvent class 事件核心同步
 */
class HCATNET_EXPORT HCatSyncCoreEvent
{
public:
    explicit HCatSyncCoreEvent();
    explicit HCatSyncCoreEvent(const EventCoreWeakPtr &core);
    ~HCatSyncCoreEvent();

    ///< 阻塞式运行事件核心
    bool Exec();

    ///< 开辟个线程运行, 非阻塞式
    bool Run();

    ///< 退出事件
    void Quit();

    ///< 获取事件核心
    const EventCorePtrType &GetEventCore() const;

private:
    std::unique_ptr<ICatSyncCoreEvent> sync_;
};


}


#endif // HCATSYNCCOREEVENT_H
