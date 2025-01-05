

#ifndef __HCATEVENTSTRUCT_H__
#define __HCATEVENTSTRUCT_H__


#include <functional>
#include <memory>

#include <HCatAny.h>
#include <HCatEventInfo.h>


namespace cat
{


///< 事件处理器类型
enum kCatEventType {
    kMinEventLimit = -1,///< 最小数量限制
    kMainEvent,         ///< 主事件, 用于提供主进程调用回调任务
    kThreadTaskEvent,   ///< 线程任务, 提供线程来执行回调任务
    kTimeEvent,         ///< 时间事件
    kIOEvent,           ///< IO事件
    kIOReadWriteEvent,  ///< IO读写事件(IO事件回调单独一个线程)
    kGcEvent,           ///< GC延迟回收事件
    kMaxEventLimit,     ///< 最大数量限制
};

///< 事件操作
enum kCatEventOperator {
    kRegister,          ///< 注册事件
    kMainRegister,      ///< 注册事件, 转发到主任务, 常用于延迟
    kCall,              ///< 调用操作
    kCallEnd,           ///< 回调结束, 返回调用者
    kIOReadCall,        ///< IO读调用, 读优先, 持续性 (IO读写单独一个线程)
    kIOWriteCall,       ///< IO写调用, 权重比读低, 单次(IO读写单独一个线程)
    kUpdateData,        ///< 查询获取或设置数据
};

struct HCatUpdateDataEvent
{
    enum eUpdateEvent {
        kIsCreateThread,        ///< 获取当前线程是否时创建线程(bool)
        kIsNeedSyncEvent,       ///< 获取是否需要同步事件(bool)
        kUpdateMainThread,      ///< 更新当前进程为主进程
        kGetIOReadWriteCount,   ///< 获取IO读写事件的当前数量
    };

    eUpdateEvent query;             ///< 查询事件
    std::shared_ptr<HCatAny> data;  ///< 查询后的返回数据
    explicit HCatUpdateDataEvent(eUpdateEvent query_) : query(query_), data(new HCatAny()) {}
};


///< 事件生存id存储的信息
struct HCatEventObjectInfo
{
    enum eDeathEvent {
        kNormal     = 0x0000,   ///< 正常状态
        kDeathIO    = 0x0001,   ///< IO事件要求删除
    };

    int deathEvent; ///< 死亡通知

    HCatEventObjectInfo() : deathEvent(kNormal) {}
    bool IsDeath() const { return deathEvent != kNormal; }
    bool IsDeathEvent(eDeathEvent status) const { return (deathEvent & status) == status; }
    void SetDeathStatus(eDeathEvent status) { deathEvent = status; }
};
using HCatEventObjectInfoPtr = std::shared_ptr<HCatEventObjectInfo>;

///< 事件生存id
struct HCatEventId
{
public:
    HCatEventId() {}
    explicit HCatEventId(HCatEventId &&d) : data(std::move(d.data)) {}
    explicit HCatEventId(const std::weak_ptr<HCatEventObjectInfo> &d) : data(d) {}
    HCatEventId(const HCatEventId &other) : data(other.data) {}
    ~HCatEventId() {}

    bool IsInvalid() const { return data.expired(); }

    ///< 检查事件是不是死亡了
    bool IsDeathEvent(HCatEventObjectInfo::eDeathEvent status) const {
        auto p = data.lock();
        if (!p) {
            return true;
        }
        return p->IsDeathEvent(status);
    }
    bool IsDeathEvent() const {
        auto p = data.lock();
        if (!p) {
            return true;
        }
        return p->IsDeath();
    }

    HCatEventId &operator= (const HCatEventId &other) { data = other.data; return *this; }

private:
    std::weak_ptr<HCatEventObjectInfo> data;
};



///< 事件单元, 传递事件
struct HCatEventUnit
{
    enum eCall {
        kMain,      ///< 转发到事件进程调用
        kCall,      ///< 转发到自身事件调用
    };

    hint32 eventType;               ///< 事件类型
    hint32 optEvent;                ///< 操作事件
    std::function<void()> callback; ///< 回调事件
    HCatEventId id;                 ///< 事件id, 控制生存期
    HCatAny data;                   ///< 携带当前事件所需数据
    eCall callRun;                  ///< 回调运行在线程还是创建事件的进程亦或者是事件自身

    explicit HCatEventUnit() : eventType(kMinEventLimit), optEvent(kRegister), callRun(kMain) {}
    explicit HCatEventUnit(int event) : eventType(event), optEvent(kRegister), callRun(kMain) {}
    explicit HCatEventUnit(int event, int opt) : eventType(event), optEvent(opt), callRun(kMain) {}
    explicit HCatEventUnit(int event, int opt, eCall call) : eventType(event), optEvent(opt), callRun(call) {}
    explicit HCatEventUnit(HCatEventUnit &&other) : eventType(other.eventType), optEvent(other.optEvent), callback(std::move(other.callback)), id(std::move(other.id)), data(std::move(other.data)), callRun(other.callRun) {}
    virtual ~HCatEventUnit(){}
};
using HCatEventUnitPtr = std::shared_ptr<HCatEventUnit>;

using HCatEventObjectPtr = std::weak_ptr<HCatEventObjectInfo>;
///< 生存对象
class HCATEVENT_EXPORT HCatEventObject
{
public:
    using PtrType = std::shared_ptr<HCatEventObjectInfo>;

    HCatEventObject() : id_(new HCatEventObjectInfo) {}
    virtual ~HCatEventObject() { id_.reset(); }

protected:
    void SetDeathEvent(HCatEventObjectInfo::eDeathEvent event) { if (id_) { id_->SetDeathStatus(event); } }

    ///< 使事件失效, 这会使继承这个对象的所有的事件都失效
    void RebirthID() { id_.reset(new HCatEventObjectInfo);}

private:
    friend class ICatEventApi;
    HCatEventObjectPtr Object() { return id_; }

private:
    PtrType id_;
};

///< 时间事件携带单元数据
struct HTimeEventUnit
{
    hint64 targetTime;  ///< 目标时间(us)
    hint64 currTime;    ///< 当前时间(us)
    bool loop;          ///< 循坏调用
    HTimeEventUnit(hint64 time, bool loop_) : targetTime(time), currTime(0), loop(loop_)  {}
};


///< 流量计数
struct HFlowCount
{
    hint64 send;    ///< 发送量
    hint64 read;    ///< 读取量
    cat::HCatAny userData;

    HFlowCount() : send(0), read(0) {}
    HFlowCount &Add(hint64 send_ = 0, hint64 read_ = 0) {
        if (send_ > 0) {
            send += send_;
        }
        if (read_ > 0) {
            read += read_;
        }
        return *this;
    }
};


}


///< IO事件携带的数据类型
typedef int H_IOType;

///< 时间事件携带数据类型
typedef cat::HTimeEventUnit H_TimeType;


#endif // HEVENTSTRUCT_H
