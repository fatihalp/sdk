

#include <HCatNet/HCatTcpTlsSocket.h>
#include <HCatNet/HCatTcpSocket.h>
#include <HCatTool.h>
#include <ICatEventApi.h>

#include <cstring>
#include <regex>

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

#define SOCKET_ERROR (-1)
#endif


#define BUFF_MAX    (1024 * 20)


using namespace cat;


HCatTcpTlsSocket::HCatTcpTlsSocket(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , port_(0)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}


HCatTcpTlsSocket::~HCatTcpTlsSocket()
{
    threadExit_ = true;
    this->DestroyMember();
    thread_.Quit();
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}


bool HCatTcpTlsSocket::Connect(const std::string &ip, huint16 port, bool threadRun, const std::string &bindIp, huint16 bindPort)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    if (ip.empty() || HCatTcpSocket::IsVaildAddr(ip) == false) {
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
        thread_.Run(this, &HCatTcpTlsSocket::ParseConnect, ip, port, addrSrv);
        return true;
    }

    this->ParseConnect(ip, port, addrSrv);
    return this->IsConnect();
}

void HCatTcpTlsSocket::Disconnect(bool signal)
{
    this->Close(signal);
}

bool HCatTcpTlsSocket::IsConnect() const
{
    return this->socket_ != SOCKET_ERROR;
}

bool HCatTcpTlsSocket::SendData(const HCatBuffer &data)
{
    if (this->socket_ == SOCKET_ERROR) {
        return false;
    }

    ICatEventApi::AddIOWrite(core_, this, &HCatTcpTlsSocket::ParseSendData, data);
    return this->IsConnect();
}

bool HCatTcpTlsSocket::GetInfo(std::string &ip, huint16 &port)
{
    if (socket_ == SOCKET_ERROR) {
        return false;
    }

    struct sockaddr_in addr;
    int addrLen = sizeof(addr);

    getsockname(socket_, (struct sockaddr *)&addr, &addrLen);
    ip = inet_ntoa(addr.sin_addr);
    port = ntohs(addr.sin_port);
    return true;
}


void HCatTcpTlsSocket::InitMember()
{
#ifdef H_WIN
    WORD ver = MAKEWORD(2, 2);
    WSAStartup(ver, &wsaData_);
#endif
}

void HCatTcpTlsSocket::DestroyMember()
{
    this->Close(false);
#ifdef H_WIN
    WSACleanup();
#endif
}

void HCatTcpTlsSocket::Close(bool updateSignal, bool resetId)
{
    if (this->socket_ == SOCKET_ERROR) {
        return ;
    }

    if (resetId) {
        this->RebirthID();
    } else {
        this->SetDeathEvent(HCatEventObjectInfo::kDeathIO);
    }

    if (ssl_) {
        HCatOpenSSL::SSLDisconnect(ssl_);
    }
    ssl_.reset();
    ctx_.reset();

#ifdef H_WIN
    closesocket(socket_);
#else
    close(socket_);
#endif

    this->socket_ = SOCKET_ERROR;

    if (updateSignal) {
        ICatEventApi::ForwardSignal(core_, this, Disconnected);
    }
}

void HCatTcpTlsSocket::DisposeEvent(hint32 fd)
{
    RallPoint<bool> threadRun(&threadRun_, true, false);
    if (threadExit_) {
        return ;
    }

    if (fd != this->socket_) {
        ParseError("parse read event error");
        return ;
    }

    HCatBuffer readData;
    do {
        hint32 size = OpenSSl()->Read(ssl_, readData);
        if (threadExit_) {
            return ;
        }
        if (size > 0) {
            break;
        } else if (size == HCatOpenSSL::kShutdown) {
            this->Close(true, false);
            return ;
        }

        if (size == HCatOpenSSL::kRetryFaild) {
            return ;
        }

        this->NotifyError();
        this->Close(true, false);
        return ;
    } while(false);

    ICatEventApi::ForwardSignal(core_, this, ReadData, readData);
}

void HCatTcpTlsSocket::ParseSendData(const HCatBuffer &data)
{
    RallPoint<bool> threadRun(&threadRun_, true, false);
    if (data.Empty()) {
        return ;
    }

    if (threadExit_) {
        return ;
    }

    if (socket_ == SOCKET_ERROR) {
        ParseError("socket not open");
        return ;
    }

    hint64 size = OpenSSl()->Write(ssl_, data);
    if (threadExit_) {
        return ;
    }

    if (size != static_cast<hint64>(data.Size())) {
        std::string log;
        this->NotifyError(&log);
        this->Close(true);
        this->ParseError(log);
    }
}

void HCatTcpTlsSocket::ParseConnect(const std::string &ip, huint16 port, const HCatAny &data)
{
    if (data.IsType<sockaddr_in>() == false) {
        ParseError("parse connect iconv type error");
        return ;
    }
    sockaddr_in addrSrv = data.Cast<sockaddr_in>();

    int result = connect(socket_, (struct sockaddr *)&addrSrv, sizeof(addrSrv));
    if (threadExit_) {
        return ;
    }

    if (result < 0) {
        if (socket_ != SOCKET_ERROR) {
            std::string log;
            this->NotifyError(&log);
            this->Close(!this->IsConnect());
            this->ParseError(log);
        }
        return ;
    }

    this->ip_ = ip;
    this->port_ = port;
    this->ParseConnectSignal();
}

bool HCatTcpTlsSocket::ParseConnectSignal()
{
    if (core_.expired()) {
        return false;
    }

    this->RebirthID();
    HCatTcpSocket::SetSocketBlock(socket_);
    ctx_ = HCatOpenSSL::CreateClientMethodContent();
    if (!ctx_) {
        this->Close();
        return false;
    }

    ssl_ = HCatOpenSSL::CreateSSL(ctx_);
    if (!ssl_) {
        this->Close();
        ParseError("Create SSL faild");
        return false;
    }

    if (!HCatOpenSSL::SetSSLFd(ssl_, socket_)) {
        this->Close();
        ParseError("SSL Set fd faild");
        return false;
    }

    if (HCatOpenSSL::SSLConnect(ssl_) != 1) {
        this->Close();
        ParseError("SSL Connect faild");
        return false;
    }

    ICatEventApi::ForwardSignal(core_, this, Connected);
    ICatEventApi::AddIO(core_, socket_, this, &HCatTcpTlsSocket::DisposeEvent, socket_);

    return true;
}

void HCatTcpTlsSocket::ParseError(const std::string &error)
{
    if (error.empty()) {
        return ;
    }

    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}

void HCatTcpTlsSocket::NotifyError(std::string *log)
{
    std::string err = HCatTool::GetError();
    if (log) {
        *log = err;
        return ;
    }

    ParseError(err);
}
