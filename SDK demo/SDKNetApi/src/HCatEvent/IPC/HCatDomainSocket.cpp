

#include <IPC/HCatDomainSocket.h>
#include <HCatNet/HCatTcpSocket.h>

#include <ICatEventApi.h>
#include <HCatTool.h>
#include <cstring>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_ERROR    (-1)
#define BUFF_MAX        (1024 * 10)


using namespace cat;


HCatDomainSocket::HCatDomainSocket(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , threadExit_(false)
    , threadRun_(false)
{

}

HCatDomainSocket::HCatDomainSocket(const EventCoreWeakPtr &core, int socket)
    : core_(core)
    , socket_(socket)
    , threadExit_(false)
    , threadRun_(false)
{
    HCatTcpSocket::SetSocketBlock(socket_);
}

HCatDomainSocket::~HCatDomainSocket()
{
    threadExit_ = true;
    this->Close(false);
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

bool HCatDomainSocket::Connect()
{
    if (SOCKET_ERROR == socket_) {
        return false;
    }

    if (core_.expired()) {
        return false;
    }

    HCatTcpSocket::SetSocketBlock(socket_);
    ICatEventApi::AddIO(core_, socket_, this, &HCatDomainSocket::DisposeEvent, socket_);
    return true;
}

bool HCatDomainSocket::Conncet(const std::string &path)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    this->Close(false);
    this->socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (this->socket_ < 0) {
        ParseError("Create domain socket is failed");
        return false;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path.data());

    if (connect(this->socket_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        this->Close(false);
        ParseError("Not connect path[" + path + "]");
        return false;
    }

    this->RebirthID();
    HCatTcpSocket::SetSocketBlock(socket_);
    ICatEventApi::AddIO(core_, socket_, this, &HCatDomainSocket::DisposeEvent, socket_);
    return true;
}

void HCatDomainSocket::Disconnect(bool signal)
{
    this->Close(signal);
}

bool HCatDomainSocket::IsConnect() const
{
    return this->socket_ != SOCKET_ERROR;
}

bool HCatDomainSocket::SendData(const HCatBuffer &data)
{
    if (this->socket_ == SOCKET_ERROR) {
        return false;
    }

    ICatEventApi::AddIOWrite(core_, this, &HCatDomainSocket::ParseSendData, data);
    return this->IsConnect();
}

void HCatDomainSocket::Close(bool updateSignal, bool resetId)
{
    if (this->socket_ == SOCKET_ERROR) {
        return ;
    }

    if (resetId) {
        this->RebirthID();
    } else {
        this->SetDeathEvent(HCatEventObjectInfo::kDeathIO);
    }

    close(socket_);
    this->socket_ = SOCKET_ERROR;

    if (updateSignal) {
        ICatEventApi::ForwardObjFunc(core_, true, this, [this](){ Disconnected(this); });
    }
}

void HCatDomainSocket::DisposeEvent(hint32 fd)
{
    RallPoint<bool> threadRun(&threadRun_, true, false);
    if (threadExit_) {
        return ;
    }

    if (fd != this->socket_) {
        ParseError("parse read event error");
        return ;
    }

    std::vector<char> data(BUFF_MAX, '\0');
    hint32 size = 0;
    for (;;) {
        if (threadExit_) {
            return ;
        }

        size = recv(this->socket_, data.data(), data.size(), MSG_NOSIGNAL);

        if (size > 0) {
            break;
        } else if (size == 0) {
            this->Close(true, false);
            return ;
        } else {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                return ;
            }
            this->NotifyError();
            this->Close(true, false);
            return ;
        }
    }

    HCatBuffer readData(data.data(), size);
    ICatEventApi::ForwardObjFunc(core_, true, this, [this, readData](){ ReadData(readData, this); });
}

void HCatDomainSocket::ParseSendData(const HCatBuffer &data)
{
    RallPoint<bool> threadRun(&threadRun_, true, false);
    if (threadExit_) {
        return ;
    }

    if (socket_ == SOCKET_ERROR) {
        ParseError("socket not open");
        return ;
    }

    std::size_t offset = 0;
    for (;;) {
        if (threadExit_) {
            return ;
        }

        hint32 sendSize = send(socket_, data.ConstData() + offset, data.Size() - offset, MSG_NOSIGNAL);
        if (sendSize > 0) {
            offset += sendSize;
            if (offset == data.Size()) {
                return ;
            }
        }

        if (sendSize == 0) {
            this->Close(true);
            return ;
        }

        if (sendSize < 0) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                continue;
            }

            std::string log;
            this->NotifyError(&log);
            this->Close(true);
            this->ParseError(log);
            return ;
        }
    }
}

void HCatDomainSocket::ParseError(const std::string &error)
{
    if (error.empty()) {
        return ;
    }

    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}

void HCatDomainSocket::NotifyError(std::string *log)
{
    std::string err = HCatTool::GetError();
    if (log) {
        *log = err;
        return ;
    }

    ParseError(err);
}
