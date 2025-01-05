

#ifndef __HCATIOREADWRITEEVENT_H__
#define __HCATIOREADWRITEEVENT_H__



#include <ICatEventBase.h>
#include <HCatThread.h>

#include <list>
#include <mutex>


namespace cat
{



class HCatIOReadWriteEvent : public ICatEventBase
{
public:
    explicit HCatIOReadWriteEvent(const EventCoreWeakPtr &core);
    ~HCatIOReadWriteEvent();
    virtual void Move(H_EventPtr &&event) override;

private:
    void NotifyRun();
    void PushWrite(H_EventPtr &&task);
    void PushRead(H_EventPtr &&task);
    void ParseQueryEvent(H_EventPtr &&event);

    void Loop();
    void ParseTask(std::list<H_EventPtr> &&task);

    ///< 执行一次读队列操作
    void RunOnceRead();
    void RunTask(H_EventPtr &&task);

private:
    EventCoreWeakPtr core_;
    std::list<H_EventPtr> writeList_;   ///< 写队列
    std::list<H_EventPtr> readList_;    ///< 读队列
    std::mutex lock_;
    std::unique_ptr<HCatThread> thread_;
    int taskNum_;
    bool exit_;
};


}


#endif // HCATIOREADWRITEEVENT_H
