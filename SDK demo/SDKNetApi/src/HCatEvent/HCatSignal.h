﻿

#ifndef __HCATSIGNAL_H__
#define __HCATSIGNAL_H__


#include <list>
#include <functional>
#include <memory>


namespace cat
{


///< 绑定成员函数的回调
template <typename _Ret, typename _Obj, typename... _Args>
std::function<_Ret(_Args...)> BindMember(_Obj *obj, _Ret(_Obj::*func)(_Args...))
{
    return [=](_Args &&...args) {
        return (obj->*func)(std::forward<_Args>(args)...);
    };
}

///< 绑定普通函数的回调
template <typename _Ret, typename... _Args>
std::function<_Ret(_Args...)> BindFunc(_Ret(*func)(_Args...))
{
    return [=](_Args &&...args) {
        return func(std::forward<_Args>(args)...);
    };
}


class HSlotBaseImpl
{
public:
    HSlotBaseImpl() {}
    virtual ~HSlotBaseImpl() {}
    HSlotBaseImpl(const HSlotBaseImpl &) = delete;
    HSlotBaseImpl & operator =(const HSlotBaseImpl &) = delete;
};


template <typename _Func>
class HSlotImpl : public HSlotBaseImpl
{
public:
    explicit HSlotImpl(const std::function<_Func> &func) : callback(func) {}
    std::function<_Func> callback;
};


class HSlot
{
public:
    HSlot() {}
    explicit HSlot(const std::shared_ptr<HSlotBaseImpl> &impl) : impl_(impl) {}
    explicit HSlot(std::shared_ptr<HSlotBaseImpl> &&impl) : impl_(impl) {}

    operator bool() const { return bool(impl_); }

    void Clear() { impl_.reset(); }

private:
    std::shared_ptr<HSlotBaseImpl> impl_;
};


/**
 * @brief The HSignal class 关联信号回调
 * 请注意: 信号不依赖事件模块
 * 如信号是在成员变量, 自身是有生存期的, 不能在自身回调中释放所在成员的对象
 * 否则这将导致回调中崩溃, 可使用事件的GC延迟施放
 */
template <typename _Func>
class HSignal
{
public:
    using SlotsTypeList = std::list<std::weak_ptr<HSlotImpl<_Func>>>;

public:
    ~HSignal() {
        slots_.clear();
        readySlots_.clear();
    }

    HSlot Bind(const std::function<_Func> &func) {
        auto f = std::make_shared<HSlotImpl<_Func>>(func);
        readySlots_.push_back(f);
        return HSlot(f);
    }

    template <typename _Obj, typename _ObjFunc>
    HSlot Bind(_Obj *obj, _ObjFunc func) { return Bind(BindMember(obj, func)); }

    template <typename ..._Args>
    HSlot Bind(HSignal<_Func> &sig) { return Bind(BindMember(&sig, &HSignal<_Func>::Emit<_Args...>)); }

    template <typename ..._Args>
    void Emit(_Args &&...args) {
        if (readySlots_.empty() == false) {
            slots_.splice(slots_.end(), std::move(readySlots_));
        }

        for (auto i = slots_.begin(); i != slots_.end();) {
            if (i->expired()) {
                i = slots_.erase(i);
                continue;
            }

            auto slot = i->lock();
            if (slot) {
                slot->callback(std::forward<_Args>(args)...);
            }

            ++i;
        }
    }

    template <typename ..._Args>
    void operator()(_Args &&...args) { this->Emit(std::forward<_Args>(args)...); }

    std::size_t Size() const { return slots_.size() + readySlots_.size(); }

private:
    SlotsTypeList slots_;
    SlotsTypeList readySlots_;
};




}



#endif // __HCATSIGNAL_H__
