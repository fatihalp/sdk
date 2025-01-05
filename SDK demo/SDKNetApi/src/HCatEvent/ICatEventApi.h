

#ifndef __ICATEVENTAPI_H__
#define __ICATEVENTAPI_H__


#include <HCatEventInfo.h>
#include <HCatEventCore.h>
#include <HCatSignal.h>

#include <functional>
#include <memory>


namespace cat
{


/**
 * @brief The ICatEventApi class 事件接口
 * 在对象析构时, 传递的事件回调既失效, 如要取消而对象不析构则调用 RebirthID 使其失效
 * 如果使用持续性事件, 比如循环的定时器, 或者监听io, 最好专门封装一个继承HCatEventObject的对象
 * 这个对象需要取消持续型的事件则调用继承对象的 RebirthID()方法使其失效
 * 可参考 HCatTimer.h HCatNet/HCatTcpSocket.h HCatNet/...
 */
class HCATEVENT_EXPORT ICatEventApi
{
public:
    ///< 当有同步任务时, 将用线程通知主进程可以调用同步
    static HSignal<void(const EventCoreWeakPtr &)> NotifyUpdateMainThread;

    ///< 要求主进程调用这个来完成事件同步
    static void MainThreadSync(const EventCoreWeakPtr &core, const std::function<void()> &eventCall = std::function<void()>(), const HCatEventObjectInfoPtr &obj = nullptr);

    ///< 查询同步信息
    static HCatAny QueryMainEvent(const EventCoreWeakPtr &core, HCatUpdateDataEvent::eUpdateEvent query, kCatEventType event = kMainEvent);

    ///< 判断是否是创建事件的线程
    static bool IsMainThread(const EventCoreWeakPtr &core);

    ///< 通知事件核心的同步进程为当前进程, 用于调整构建事件核心和运行的线程不一致
    static void SwitchMainThread(const EventCoreWeakPtr &core);

    ///< 判断是否需要同步事件
    static bool IsNeedSyncEvent(const EventCoreWeakPtr &core);

    ///< 获取IO读写任务事件数量
    static int GetIOReadWriteCount(const EventCoreWeakPtr &core);

    /**
     * @brief AddTime 添加定时器 (这需要继承 HCatEventObject) 可以使用已封装好的 HCatTimer.
     * @param core 事件核心
     * @param delay 时间以ms为单位
     * @param loop 循环定时器触发, 为true时, 如要取消而对象不析构则调用RebirthID使其失效
     * @param threadRun 在线程上调用
     * @param obj 对象
     * @param func 对象函数
     * @param args 回调参数
     */
    template <typename _Obj, typename _Func = typename _Obj::_Func, typename _Base = typename _Obj::PtrType, typename... _Args>
    static void AddTime(const EventCoreWeakPtr &core, hint64 delay, bool loop, _Obj *obj, _Func func, _Args &&...args) {
        auto f = std::bind(func, obj, std::forward<_Args>(args)...);
        return CreateEventSelectThread(core, kTimeEvent, kRegister, std::move(f), H_TimeType(delay * 1000, loop), obj->Object());
    }
    template <typename _Obj, typename _Base = typename _Obj::PtrType>
    static void AddTime(const EventCoreWeakPtr &core, hint64 delay, bool loop, _Obj *obj, std::function<void()> &&func) {
        return CreateEventSelectThread(core, kTimeEvent, kRegister, std::move(func), H_TimeType(delay * 1000, loop), obj->Object());
    }


    /**
     * @brief AddIO 将io添加到事件检测, 将会在IO读写的线程进行回调(这需要继承 HCatEventObject), 如要取消而对象不析构则调用RebirthID使其失效
     * @param core 事件核心
     * @param fd 文件描述符
     * @param obj 对象
     * @param func 对象函数
     * @param args 自定义参数
     */
    template <typename _Obj, typename _Func = typename _Obj::_Func, typename _Base = typename _Obj::PtrType, typename... _Args>
    static void AddIO(const EventCoreWeakPtr &core, H_IOType fd, _Obj *obj, _Func func, _Args &&...args) {
        auto f = std::bind(func, obj, std::forward<_Args>(args)...);
        CreateEventData(core, kIOEvent, kRegister, std::move(f), fd, obj->Object());
    }
    template <typename _Obj, typename _Base = typename _Obj::PtrType>
    static void AddIO(const EventCoreWeakPtr &core, H_IOType fd, _Obj *obj, std::function<void(H_IOType)> &&func) {
        auto f = [fd, func]() { func(fd); };
        CreateEventData(core, kIOEvent, kRegister, std::move(f), fd, obj->Object);
    }

    /**
     * @brief AddIOWrite IO读写需要在独立的一个线程, 写操作需要在指定线程运转, 会在指定线程回调(这需要继承 HCatEventObject)
     * @param core 事件核心
     * @param obj 对象
     * @param func 对象函数
     * @param args 自定义参数
     */
    template <typename _Obj, typename _Func = typename _Obj::_Func, typename _Base = typename _Obj::PtrType, typename... _Args>
    static void AddIOWrite(const EventCoreWeakPtr &core, _Obj *obj, _Func func, _Args &&...args) {
        auto f = std::bind(func, obj, std::forward<_Args>(args)...);
        CreateEventFunc(core, kIOReadWriteEvent, kIOWriteCall, std::move(f), obj->Object());
    }
    template <typename _Obj, typename _Base = typename _Obj::PtrType>
    static void AddIOWrite(const EventCoreWeakPtr &core, _Obj *obj,  std::function<void()> &&func) {
        CreateEventFunc(core, kIOReadWriteEvent, kIOWriteCall, std::move(func), obj->Object());
    }

    /**
     * @brief AddThread 添加任务到线程池执行(这需要继承 HCatEventObject)
     * @param userData 自身用于检测生存期
     * @param core 事件核心
     * @param obj 对象
     * @param func 对象函数
     * @param args 参数
     */
    template <typename _Obj, typename _Func = typename _Obj::_Func, typename _Base = typename _Obj::PtrType, typename... _Args>
    static void AddThread(const EventCoreWeakPtr &core, HCatAny &&userData, _Obj *obj, _Func func, _Args &&...args) {
        auto f = std::bind(func, obj, std::forward<_Args>(args)...);
        CreateEventFunc(core, kThreadTaskEvent, kRegister, std::move(f), obj->Object(), std::move(userData));
    }
    template <typename _Obj, typename _Base = typename _Obj::PtrType>
    static void AddThread(const EventCoreWeakPtr &core, HCatAny &&userData, _Obj *obj, std::function<void()> &&func) {
        CreateEventFunc(core, kThreadTaskEvent, kRegister, std::move(func), obj->Object(), std::move(userData));
    }

    /**
     * @brief Forward 转发到同步主进程(这需要继承 HCatEventObject)
     * @param core 事件核心
     * @param obj 对象
     * @param func 对象函数
     * @param args 参数
     * @return 返回事件id
     */
    template <typename _Obj, typename _Func = typename _Obj::_Func, typename _Base = typename _Obj::PtrType, typename... _Args>
    static void Forward(const EventCoreWeakPtr &core, bool directMainCall, _Obj *obj, _Func func, _Args &&...args) {
        HCatEventObjectPtr object =  obj->Object();
        auto f = std::bind(func, obj, std::forward<_Args>(args)...);
        if (directMainCall && ICatEventApi::IsMainThread(core)) {
            f();
            return ;
        }
        CreateEventFunc(core, kMainEvent, kCall, std::move(f), std::move(object));
    }
    template <typename _Obj, typename _Base = typename _Obj::PtrType>
    static void ForwardObjFunc(const EventCoreWeakPtr &core, bool directMainCall, _Obj *obj, std::function<void()> &&func) {
        HCatEventObjectPtr object =  obj->Object();
        if (directMainCall && ICatEventApi::IsMainThread(core)) {
            func();
            return ;
        }
        CreateEventFunc(core, kMainEvent, kCall, std::move(func), std::move(object));
    }


    ///< 转发信号, 这需要继承 HCatEventObject
    template <typename _Obj, typename _Func, typename _Base = typename _Obj::PtrType, typename ..._Args>
    static void ForwardSignal(const EventCoreWeakPtr &core, _Obj *obj, HSignal<_Func> &signal, _Args &&...args) {
        if (ICatEventApi::IsMainThread(core)) {
            signal(std::forward<_Args>(args)...);
            return ;
        }

        auto f = std::bind(&HSignal<_Func>::template Emit<_Args...>, &signal, std::forward<_Args>(args)...);
        ICatEventApi::Forward(core, false, obj, std::move(f));
    }

    /**
     * @brief Gc 延迟释放
     * @param core 事件核心
     * @param data 常用于RALL式数据
     * @param func 这是只能用于释放的回调, 不检测生存期, 使用这个不要关联生存期的对象
     */
    static void Gc(const EventCoreWeakPtr &core, HCatAny &&data, std::function<void()> &&func = std::function<void()>()) {
        CreateEventData(core, kGcEvent, kMainRegister, std::move(func), std::move(data));
    }

private:
    static void CreateEventSelectThread(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func, HCatAny &&data, HCatEventObjectPtr &&obj);

    static void CreateEventFunc(const EventCoreWeakPtr &core, int event, int opt, std::function<void()> &&func, HCatEventObjectPtr &&obj, HCatAny &&userData = HCatAny());

    ///< 这是一个特殊的没有对象的, 用于事件不检测生存期不回调的那种
    static void CreateEventData(const EventCoreWeakPtr &core, int event, int opt, std::function<void()> &&func, HCatAny &&data);

    static void CreateEventData(const EventCoreWeakPtr &core, int event, int opt, std::function<void()> &&func, HCatAny &&data, HCatEventObjectPtr &&obj);
};


}


#endif // __ICATEVENTAPI_H__
