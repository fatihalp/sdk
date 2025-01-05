

#ifndef __ICATSYNCCOREEVENT_H__
#define __ICATSYNCCOREEVENT_H__


#include <ICatEventApi.h>


namespace cat
{

class ICatSyncCoreEventPrivate;

class HCATEVENT_EXPORT ICatSyncCoreEvent
{
public:
    explicit ICatSyncCoreEvent(const EventCoreWeakPtr &core);
    ~ICatSyncCoreEvent();

    ///< 阻塞式运行事件核心
    bool Exec();

    ///< 开辟个线程运行, 非阻塞式
    bool Run();

    ///< 退出事件
    void Quit();

    ///< 获取事件核心
    const EventCorePtrType &GetEventCore() const;

private:
    friend class ICatSyncCoreEventPrivate;
    std::unique_ptr<ICatSyncCoreEventPrivate> d_;
};


}


#endif // __ICATSYNCCOREEVENT_H__
