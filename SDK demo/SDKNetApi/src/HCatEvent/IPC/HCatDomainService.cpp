

#include <IPC/HCatDomainService.h>
#include <ICatEventApi.h>
#include <HCatTool.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_ERROR (-1)


using namespace cat;


HCatDomainService::HCatDomainService(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , threadExit_(false)
    , threadRun_(false)
{

}

HCatDomainService::~HCatDomainService()
{
    threadExit_ = true;
    this->Close(false);
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

bool HCatDomainService::Listen(const std::string &path)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    this->Close(false);

    this->socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (this->socket_ < 0) {
        ParseError("create socket is failed");
        return false;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path.data());
    unlink(path.data());

    if (bind(this->socket_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        this->Close(false);
        ParseError("not bind domain");
        return false;
    }

    if (listen(this->socket_, SOMAXCONN) < 0) {
        this->Close(false);
        ParseError("listen is failed");
        return false;
    }

    this->RebirthID();
    Connected(this);
    ICatEventApi::AddIO(core_, socket_, this, &HCatDomainService::DisposeEvent, socket_);
    return true;

}

bool HCatDomainService::IsListen() const
{
    return this->socket_ != SOCKET_ERROR;
}

void HCatDomainService::Close(bool signal)
{
    if (socket_ == SOCKET_ERROR) {
        return ;
    }

    this->RebirthID();
    close(socket_);
    this->socket_ = SOCKET_ERROR;

    if (signal) {
        ICatEventApi::ForwardObjFunc(core_, true, this, [this](){ Disconnected(this); });
    }
}

void HCatDomainService::DisposeEvent(int fd)
{
    RallPoint<bool> threadRun(&threadRun_, true, false);
    if (threadExit_) {
        return ;
    }

    if (this->socket_ != fd) {
        return ;
    }

    hint32 client = accept(socket_, nullptr, nullptr);
    if (threadExit_) {
        return ;
    }

    if (client == SOCKET_ERROR) {
        std::string log;
        this->NotifyError(&log);
        this->Close();
        this->ParseError(log);
        return ;
    }

    ICatEventApi::Forward(core_, true, this, &HCatDomainService::ParseNewConnect, client);
}

void HCatDomainService::NotifyError(std::string *log)
{
    std::string err = HCatTool::GetError();
    if (log) {
        *log = err;
        return ;
    }

    ParseError(err);
}

void HCatDomainService::ParseError(const std::string &error)
{
    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}

void HCatDomainService::ParseNewConnect(int fd)
{
    std::shared_ptr<HCatDomainSocket> socketClient(new HCatDomainSocket(core_, fd));
    NewConnect(socketClient, this);
    socketClient->Connect();
}
