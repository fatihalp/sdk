

#include <HCatNet/HCatTcpSocket.h>
#include <HCatTool.h>
#include <ICatEventApi.h>

#include <cstring>
#include <regex>
#include <vector>

#ifdef H_WIN
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define SOCKET_ERROR (-1)
#endif


#define BUFF_MAX    (1024 * 20)


using namespace cat;


HCatTcpSocket::HCatTcpSocket(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , port_(0)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}

HCatTcpSocket::HCatTcpSocket(const EventCoreWeakPtr &core, int socket, const std::string &ip, huint16 port)
    : core_(core)
    , socket_(socket)
    , ip_(ip)
    , port_(port)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}

HCatTcpSocket::~HCatTcpSocket()
{
    threadExit_ = true;
    this->DestroyMember();
    thread_.Quit();
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

bool HCatTcpSocket::Connect()
{
    if (SOCKET_ERROR == socket_) {
        return false;
    }

    return this->ParseConnectSignal(false);
}

bool HCatTcpSocket::Connect(const std::string &ip, huint16 port, bool threadRun, const std::string &bindIp, huint16 bindPort)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    if (ip.empty() || IsVaildAddr(ip) == false) {
        ParseError("Invalid Address");
        return false;
    }

    this->Close(false);

    this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == socket_){
        ParseError("create socket faild");
        return false;
    }

#ifdef H_WIN
    char opt = 1;
#else
    int opt = 1;
#endif
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

#ifdef SO_REUSEPORT
    opt = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

    if (bindPort != 0) {
        struct sockaddr_in bindAddr;
        memset(&bindAddr, 0, sizeof(bindAddr));
        bindAddr.sin_family = AF_INET;
        bindAddr.sin_port = htons(bindPort);
        if (bindIp.empty()) {
            bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            bindAddr.sin_addr.s_addr = inet_addr(bindIp.data());
        }
    }

    struct sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    addrSrv.sin_addr.s_addr = inet_addr(ip.data());

    if (threadRun) {
        thread_.Run(this, &HCatTcpSocket::ParseConnect, socket_, ip, port, addrSrv);
        return true;
    }

    this->ParseConnect(socket_, ip, port, addrSrv);
    return this->IsConnect();
}

void HCatTcpSocket::Disconnect(bool signal)
{
    this->Close(signal);
}

bool HCatTcpSocket::IsConnect() const
{
    return this->socket_ != SOCKET_ERROR;
}

bool HCatTcpSocket::SendData(const HCatBuffer &data)
{
    if (this->socket_ == SOCKET_ERROR) {
        return false;
    }

    ICatEventApi::AddIOWrite(core_, this, &HCatTcpSocket::ParseSendData, data);
    return this->IsConnect();
}

void HCatTcpSocket::AddFlowMonitor(const std::weak_ptr<HFlowCount> &flow)
{
    ICatEventApi::AddIOWrite(core_, this, &HCatTcpSocket::ParseAddFlow, flow);
}

bool HCatTcpSocket::GetInfo(std::string &ip, huint16 &port)
{
    if (socket_ == SOCKET_ERROR) {
        return false;
    }

    struct sockaddr_in addr;
#ifdef H_WIN
    int addrLen = sizeof(addr);
#else
    socklen_t addrLen = sizeof(addr);
#endif

    getsockname(socket_, (struct sockaddr *)&addr, &addrLen);
    ip = inet_ntoa(addr.sin_addr);
    port = ntohs(addr.sin_port);
    return true;
}

bool HCatTcpSocket::IsIpv4(const std::string &ip)
{
    std::smatch res;
    std::regex ipv4(R"(^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$)");
    std::string addr(ip.substr(0, ip.find(R"(:)")));
    return std::regex_match(addr, res, ipv4);
}

bool HCatTcpSocket::IsIpv6(const std::string &ip)
{
    std::smatch res;
    std::regex ipv6(R"(^([\da-fA-F]{1,4}:){7}[\da-fA-F]{1,4}$)");
    return std::regex_match(ip, res, ipv6);
}

std::string HCatTcpSocket::UrlToHost(const std::string &url)
{
    if (url.empty()) {
        return url;
    }

    std::size_t pos = url.find(R"(://)");

    // 检查是否存在标志头
    if (pos != std::string::npos) {
        pos += 3;
        return url.substr(pos, url.find(R"(/)", pos) - pos);
    }

    std::size_t lastPos = url.find(R"(:)");
    if (lastPos == std::string::npos) {
        return url.substr(0, url.find(R"(/)"));
    }

    return url.substr(0, lastPos);
}

std::string HCatTcpSocket::UrlToResource(const std::string &url)
{
    if (url.empty()) {
        return url;
    }

    std::size_t pos = url.find(R"(://)");

    // 检查是否存在标志头
    if (pos != std::string::npos) {
        pos += 3;
        return url.substr(url.find(R"(/)", pos));
    }

    pos = url.find(R"(/)");
    if (pos == std::string::npos) {
        return url;
    }

    return url.substr(pos);
}

std::list<std::string> HCatTcpSocket::UrlToIp(const std::string &url)
{
#ifdef H_WIN
    // 用于激活windows网络库初始化
    static std::unique_ptr<HCatTcpSocket>(new HCatTcpSocket(EventCoreWeakPtr()));
#endif

    std::list<std::string> ipList;
    std::string hostAddr = UrlToHost(url);
    struct hostent *host = gethostbyname(hostAddr.data());
    if (!host) {
        return ipList;
    }

    switch (host->h_addrtype) {
    case AF_INET:
    case AF_INET6:
        break;
    default:
        return ipList;
        break;
    }

    for(int i = 0; host->h_addr_list[i]; ++i){
        struct in_addr *addr = reinterpret_cast<struct in_addr *>(host->h_addr_list[i]);
        ipList.emplace_back(inet_ntoa(*addr));
    }

    return ipList;
}

bool HCatTcpSocket::SetSocketBlock(int socket)
{
#ifdef H_WIN
    u_long opt = 1;
    if (ioctlsocket(socket, FIONBIO, &opt)) {
        return false;
    }
#else
    int opt = fcntl(socket, F_GETFL);
    if (opt < 0) {
        return false;
    }

    opt |= O_NONBLOCK;
    if (fcntl(socket, F_SETFL, opt) < 0) {
        return false;
    }
#endif
    return true;
}

void HCatTcpSocket::InitMember()
{
#ifdef H_WIN
    WORD ver = MAKEWORD(2, 2);
    WSAStartup(ver, &wsaData_);
#endif
}

void HCatTcpSocket::DestroyMember()
{
    this->Close(false);
#ifdef H_WIN
    WSACleanup();
#endif
}

void HCatTcpSocket::Close(bool updateSignal, bool resetId)
{
    if (this->socket_ == SOCKET_ERROR) {
        return ;
    }

    if (resetId) {
        this->RebirthID();
    } else {
        this->SetDeathEvent(HCatEventObjectInfo::kDeathIO);
    }

#ifdef H_WIN
    closesocket(socket_);
#else
    close(socket_);
#endif

    this->socket_ = SOCKET_ERROR;

    if (updateSignal) {
        ICatEventApi::ForwardObjFunc(core_, true, this, [this](){ Disconnected(this); });
    }
}

void HCatTcpSocket::DisposeEvent(hint32 fd)
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

#ifdef MSG_NOSIGNAL
        size = recv(this->socket_, data.data(), data.size(), MSG_NOSIGNAL);
#else
        size = recv(this->socket_, data.data(), data.size(), 0);
#endif

        if (size > 0) {
            break;
        } else if (size == 0) {
            this->Close(true, false);
            return ;
        } else {
#ifdef H_WIN
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
#endif
                return ;
            }
            this->NotifyError();
            this->Close(true, false);
            return ;
        }
    }

    AddFlow(0, size);
    HCatBuffer readData(data.data(), size);
    ICatEventApi::ForwardObjFunc(core_, true, this, [this, readData](){ ReadData(readData, this); });
}

void HCatTcpSocket::ParseSendData(const HCatBuffer &data)
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

#ifdef MSG_NOSIGNAL
        hint32 sendSize = send(socket_, data.ConstData() + offset, data.Size() - offset, MSG_NOSIGNAL);
#else
        hint32 sendSize = send(socket_, data.ConstData() + offset, data.Size() - offset, 0);
#endif
        AddFlow(sendSize);
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
#ifdef H_WIN
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
#endif
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

void HCatTcpSocket::ParseConnect(socketType socket, const std::string &ip, huint16 port, const HCatAny &data)
{
    if (data.IsType<sockaddr_in>() == false) {
        ParseError("parse connect iconv type error");
        return ;
    }
    sockaddr_in addrSrv = data.Cast<sockaddr_in>();

    int result = connect(socket, (struct sockaddr *)&addrSrv, sizeof(addrSrv));
    if (threadExit_) {
        return ;
    }

    if (result < 0) {
        if (socket_ != SOCKET_ERROR) {
            std::string log;
            this->NotifyError(&log);
            this->Close(socket == socket_);
            this->ParseError(log);
        }
        return ;
    }

    this->ip_ = ip;
    this->port_ = port;
    this->ParseConnectSignal();
}

bool HCatTcpSocket::ParseConnectSignal(bool resetId)
{
    if (core_.expired()) {
        return false;
    }

    if (resetId) {
        this->RebirthID();
    }
    SetSocketBlock(socket_);
    ICatEventApi::ForwardObjFunc(core_, true, this, [this](){ Connected(this); });
    ICatEventApi::AddIO(core_, socket_, this, &HCatTcpSocket::DisposeEvent, socket_);

    return true;
}

void HCatTcpSocket::ParseError(const std::string &error)
{
    if (error.empty()) {
        return ;
    }

    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}

void HCatTcpSocket::NotifyError(std::string *log)
{
    std::string err = HCatTool::GetError();
    if (log) {
        *log = err;
        return ;
    }

    ParseError(err);
}

void HCatTcpSocket::ParseAddFlow(const std::weak_ptr<HFlowCount> &flow)
{
    if (flow.expired()) {
        return ;
    }
    flowCount_.push_back(flow);
}

void HCatTcpSocket::AddFlow(hint64 send, hint64 read)
{
    for (auto i = flowCount_.begin(); i != flowCount_.end();) {
        auto flow = i->lock();
        if (!flow) {
            i = flowCount_.erase(i);
            continue;
        }

        flow->Add(send, read);
        ++i;
    }
}
