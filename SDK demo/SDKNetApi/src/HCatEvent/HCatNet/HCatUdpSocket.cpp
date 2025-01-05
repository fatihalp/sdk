

#include <HCatNet/HCatUdpSocket.h>
#include <ICatEventApi.h>
#include <string.h>
#include <HCatTool.h>

#ifdef H_WIN
#include <HCatTool.h>
#include <WinSock2.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <Iphlpapi.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#define SOCKET_ERROR (-1)
#endif


#define BUFF_MAX    (1024 * 20)


using namespace cat;



HCatUdpSocket::HCatUdpSocket(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , port_(0)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}

HCatUdpSocket::~HCatUdpSocket()
{
    threadExit_ = true;
    this->DestroyMember();
    this->RebirthID();
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

HCatUdpSocket::IpType HCatUdpSocket::IP(const std::string &ip, eUdpSendType type)
{
    switch (type) {
    case kSendUnicast: {
        return inet_addr(ip.data());
    } break;
    case kSendMulticast: {
        return INADDR_ANY;
    } break;
    case kSendBroadcast: {
        return INADDR_BROADCAST;
    } break;
    default:
        return INADDR_ANY;
    }
}

std::list<std::string> HCatUdpSocket::GetDeviceIP(const std::string &device)
{
    std::list<std::string> ipList;
    if (device.empty()) {
        return ipList;
    }

#ifdef H_WIN
    //PIP_ADAPTER_INFO结构体指针存储本机网卡信息
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    //得到结构体大小,用于GetAdaptersInfo参数
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
    //记录网卡数量
    int netCardNum = 0;
    //记录每张网卡上的IP地址数量
    int IPnumPerNetCard = 0;

    if (ERROR_BUFFER_OVERFLOW == nRel) {
        //如果函数返回的是ERROR_BUFFER_OVERFLOW
        //则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
        //这也是说明为什么stSize既是一个输入量也是一个输出量
        //释放原来的内存空间
        delete pIpAdapterInfo;
        //重新申请内存空间用来存储所有网卡信息
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        //再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
        nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);
    }

    do {
        if (ERROR_SUCCESS != nRel) {
            break;
        }

        PIP_ADAPTER_INFO info = pIpAdapterInfo;
        while (info) {
            // 查询相同的适配器名
            if (std::string(info->AdapterName) != device) {
                info = info->Next;
                continue;
            }

            IP_ADDR_STRING *pIpAddrString = &(info->IpAddressList);
            // 可能网卡有多IP, 只获取一个
            while (pIpAddrString) {
                std::string ip = std::string(pIpAddrString->IpAddress.String);
                if (ip.empty() == false) {
                    ipList.push_back(std::move(ip));
                }
                pIpAddrString = pIpAddrString->Next;
            }
            break;
        }
    } while (false);

    if (pIpAdapterInfo) {
        delete pIpAdapterInfo;
    }

    return ipList;
#else

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    const int len = std::max(sizeof(ifr.ifr_name), device.size());
    strncpy(ifr.ifr_name, device.data(), len);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    do {
        if (ioctl(fd, SIOCGIFADDR, &ifr) != 0) {
            break;
        }

        struct sockaddr_in sin;
        memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
        std::string ip(inet_ntoa(sin.sin_addr));
        if (ip.empty() == false) {
            ipList.push_back(std::move(ip));
        }
    } while (false);

    close(fd);
    return ipList;
#endif
}

bool HCatUdpSocket::Connect(const std::string &sendIp, huint16 sendPort, eUdpSendType sendType, const std::string &listenIp, huint16 listentPort, bool isBind, const std::string &device)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    this->Close(false);
    this->port_ = sendPort;
    this->sendIp_ = sendIp;
    this->sendType_ = sendType;

    socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SOCKET_ERROR == socket_){
        ParseError("create socket faild");
        return false;
    }

    switch (sendType) {
    case kSendMulticast: {
        // 加入多播组
        struct ip_mreq multcast;
        multcast.imr_multiaddr.s_addr = IP(sendIp, sendType);
        multcast.imr_interface.s_addr = htonl(INADDR_ANY);
        setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char *>(&multcast), sizeof(multcast));
    } break;
    case kSendBroadcast: {
#ifdef H_WIN
        char option = 1;
#else
        int option = 1;
#endif
        setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option));
    } break;
    default:
        break;
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

    // 是否绑定
    if (isBind == false) {
        ICatEventApi::ForwardSignal(core_, this, Connected);
        return true;
    }

    sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(listentPort);
    addrSrv.sin_addr.s_addr = IP(listenIp, listenIp.empty() ? kSendMulticast : kSendUnicast);
    if (device.empty() == false) {
        std::list<std::string> ipAddr = GetDeviceIP(device);
        if (ipAddr.empty() == false) {
            addrSrv.sin_addr.s_addr = inet_addr(ipAddr.front().data());
        }
    }

    int ret = bind(socket_, reinterpret_cast<sockaddr *>(&addrSrv), sizeof(addrSrv));
    if (ret != 0) {
        std::string log;
        this->NotifyError(&log);
        this->Close();
        this->ParseError(log);
        return false;
    }

    return ParseConnectSignal();
}

void HCatUdpSocket::Disconnect(bool signal)
{
    this->Close(signal);
}

bool HCatUdpSocket::IsConnect() const
{
    return this->socket_ != SOCKET_ERROR;
}


bool HCatUdpSocket::SendData(const std::string &ip, huint16 port, const HCatBuffer &data, eUdpSendType sendType)
{
    if (this->socket_ == SOCKET_ERROR) {
        return false;
    }

    ICatEventApi::AddIOWrite(core_, this, &HCatUdpSocket::ParseSendData, ip, port, data, sendType);
    return this->IsConnect();
}

void HCatUdpSocket::NotifyError(std::string *log)
{
    std::string err = HCatTool::GetError();
    if (log) {
        *log = err;
        return ;
    }

    ParseError(err);
}

void HCatUdpSocket::InitMember()
{
#ifdef H_WIN
    WORD ver = MAKEWORD(2, 2);
    WSAStartup(ver, &wsaData_);
#endif
}

void HCatUdpSocket::DestroyMember()
{
#ifdef H_WIN
    WSACleanup();
#endif
}

void HCatUdpSocket::DisposeEvent(hint32 fd)
{
    RallPoint<bool> threadRun(&threadRun_, true, false);
    if (threadExit_) {
        return ;
    }

    if (fd != this->socket_) {
        ParseError("parse read event error");
        return ;
    }

    struct sockaddr_in addr;
#ifdef H_WIN
    int addrLen = sizeof(struct sockaddr_in);
#else
    socklen_t addrLen = sizeof(struct sockaddr_in);
#endif
    char data[BUFF_MAX];

    hint32 size = recvfrom(this->socket_, data, BUFF_MAX, 0, reinterpret_cast<sockaddr *>(&addr), &addrLen);
    if (threadExit_) {
        return ;
    }

    if (size <= 0) {
        std::string log;
        NotifyError(&log);
        Close(true);
        ParseError(log);
        return ;
    }

    HCatBuffer readData(data, size);
    std::string ip(inet_ntoa(addr.sin_addr));
    huint16 port = ntohs(addr.sin_port);
    ICatEventApi::ForwardSignal(core_, this, ReadData, ip, port, readData);
}

void HCatUdpSocket::Close(bool updateSignal)
{
    if (this->socket_ == SOCKET_ERROR) {
        return ;
    }
    this->RebirthID();

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

void HCatUdpSocket::ParseSendData(const std::string &ip, huint16 port, const HCatBuffer &data, eUdpSendType sendType)
{
    RallPoint<bool> threadRun(&threadRun_, true, false);
    if (threadExit_) {
        return ;
    }

    if (socket_ == SOCKET_ERROR) {
        ParseError("socket not open");
        return ;
    }

    sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    addrSrv.sin_addr.s_addr = IP(ip, sendType);

    hint32 size = sendto(socket_, data.ConstData(), data.Size(), 0, reinterpret_cast<sockaddr *>(&addrSrv), sizeof(addrSrv));
    if (threadExit_) {
        return ;
    }

    if (size == SOCKET_ERROR) {
        std::string log;
        this->NotifyError(&log);
        this->Close(true);
        this->ParseError(log);
    }
}

bool HCatUdpSocket::ParseConnectSignal()
{
    if (core_.expired()) {
        return false;
    }

    this->RebirthID();
    ICatEventApi::ForwardSignal(core_, this, Connected);
    ICatEventApi::AddIO(core_, socket_, this, &HCatUdpSocket::DisposeEvent, socket_);

    return true;
}

void HCatUdpSocket::ParseError(const std::string &error)
{
    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}
