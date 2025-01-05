

#ifndef __HCATDOMAINSERVICE_H__
#define __HCATDOMAINSERVICE_H__


#include <IPC/HCatDomainSocket.h>


namespace cat
{


class HCATEVENT_EXPORT HCatDomainService : public HCatEventObject
{
public:
    ///< 新连接将发送该信号, 请必须在触发这个信号后完成新连接信号绑定, 信号触发完毕后将自动进行连接的 Connect 初始化操作
    HSignal<void(std::shared_ptr<HCatDomainSocket>, HCatDomainService *)> NewConnect;

    ///< 错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

    ///< 成功连接将发送该信号
    HSignal<void(HCatDomainService *)> Connected;

    ///< 断开连接将发送该信号
    HSignal<void(HCatDomainService *)> Disconnected;

public:
    explicit HCatDomainService(const EventCoreWeakPtr &core);
    virtual ~HCatDomainService();

    bool Listen(const std::string &path);
    bool IsListen() const;
    void Close(bool signal = true);

private:
    void DisposeEvent(int fd);
    void NotifyError(std::string *log = nullptr);
    void ParseError(const std::string &error);
    void ParseNewConnect(int fd);

private:
    EventCoreWeakPtr core_;
    hint32 socket_;
    bool threadExit_;
    bool threadRun_;
};


}


#endif // __HCATDOMAINSERVICE_H__
