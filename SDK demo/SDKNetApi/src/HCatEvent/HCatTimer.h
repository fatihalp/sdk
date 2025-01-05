﻿

#ifndef __HCATTIMER_H__
#define __HCATTIMER_H__


#include <ICatEventApi.h>
#include <HCatEventStruct.h>


namespace cat
{


class HCATEVENT_EXPORT HCatTimer : public HCatEventObject
{
public:
    ///< 超时信号
    HSignal<void()> Timeout;

public:
    explicit HCatTimer() {}

    /**
     * @brief Start 开始定时器
     * @param ms 毫秒
     * @param loop 是否循环触发, 默认true
     */
    void Start(const EventCoreWeakPtr &core, hint64 ms, bool loop = true) {
        this->Stop();
        ICatEventApi::AddTime(core, ms, loop, this, &HCatTimer::ParseTimeout);
    }
    void StartLast(const EventCoreWeakPtr &core, hint64 ms, bool loop = true) {
        ICatEventApi::Forward(core, true, this, &HCatTimer::Start, core, ms, loop);
    }

    void Stop() { this->RebirthID(); }

private:
    void ParseTimeout() { Timeout(); }
};


}


#endif // __HCATTIMER_H__
