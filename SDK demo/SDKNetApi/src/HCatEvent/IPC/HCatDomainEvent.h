

#ifndef __HCATDOMAINEVENT_H__
#define __HCATDOMAINEVENT_H__


#include <IPC/HCatDomainService.h>
#include <HCatSignalSet.h>
#include <map>
#include <mutex>


namespace cat
{


class HCATEVENT_EXPORT HCatDomainEvent : public HCatEventObject
{
    ///< 提供外部的事件结构
public:
    struct HEventUnit {
        hint64 event;       ///< 指定事件

        ~HEventUnit() { signalSet.Clear(); }
    private:
        friend class HCatDomainEvent;
        HCatSignalSet signalSet;    ///< 供内部关联信号使用
    };
    struct HServiceEvent {
        HEventUnit unit;
        std::shared_ptr<HCatDomainService> service;
    };
    struct HClientEvent {
        HEventUnit unit;
        HCatBuffer buffer;    ///< 提供外部使用
        std::shared_ptr<HCatDomainSocket> client;
    };
    using HServerEventPtr = std::shared_ptr<HServiceEvent>;
    using HServerEventWeakPtr = std::weak_ptr<HServiceEvent>;
    using HClientEventPtr = std::shared_ptr<HClientEvent>;
    using HClientEventWeakPtr = std::weak_ptr<HClientEvent>;

    ///< 信号
public:
    ///< 有新客户端事件生成
    HSignal<void(hint64, HClientEventPtr)> NewClient;

    ///< 客户端断开
    HSignal<void(hint64, HClientEventPtr)> Disconnect;

    ///< 客户端读取的数据
    HSignal<void(hint64, const HCatBuffer &, HClientEventPtr)> ReadyRead;

    ///< 错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

public:
    explicit HCatDomainEvent(const EventCoreWeakPtr &core);

    /**
     * @brief Register 注册事件对象
     * @param event 事件
     * @param path Unix Socket的路径
     * @return 成功返回true, 反之返回false
     */
    bool Register(hint64 event, const std::string &path);
    void UnRegister(hint64 event);

    ///< 查删查客户端, 传入客户端指针
    HClientEventPtr FindClient(HCatDomainSocket *clientPtr);
    std::list<HClientEventPtr> FindClient(hint64 event);
    void RemoveClient(HCatDomainSocket *clientPtr);
    bool ExistClient(HCatDomainSocket *clientPtr);

    bool ExistEvent(hint64 event);

    ///< 发送指定事件的所有对象
    void SendBroadCast(hint64 event, const cat::HCatBuffer &data);

private:
    ///< 处理服务信号
    void ParseNewClient(std::shared_ptr<HCatDomainSocket> client, HCatDomainService *service);

    ///< 处理客户端信号
    void ParseClientReadData(const HCatBuffer &data, HCatDomainSocket *client);
    void ParseClientDisconnect(HCatDomainSocket *client);

    HServerEventPtr FindService(HCatDomainService *servicePtr);
private:
    EventCoreWeakPtr core_;
    std::mutex lock_;
    std::map<hint64, HServerEventPtr> service_; ///< 侦听事件
    std::map<HCatDomainSocket *, HClientEventPtr> client_;  ///< 客户端
};



}


#endif // __HCATDOMAINEVENT_H__
