#include <iostream>
#include <string>

#include "../include/ICApi.h"

#ifdef HWIN64
#pragma comment(lib,"../lib64/HCatNet.lib")
#elif HWIN32
#pragma comment(lib,"../lib/HCatNet.lib")
#endif


#define GETDEVICEINFO_TEXT          "<?xml version=\'1.0\' encoding=\'utf-8\'?>\n" \
                                    "<sdk guid=\"##GUID\">\n"                      \
                                    "    <in method=\"GetDeviceInfo\"/>\n"         \
                                    "</sdk>"

#define GETFILEINFO_TEXT          "<?xml version=\'1.0\' encoding=\'utf-8\'?>\n" \
                                    "<sdk guid=\"##GUID\">\n"                    \
                                    "    <in method=\"GetFiles\"/>\n"            \
                                    "</sdk>"

static HEventCore *core;


static void ReadData(HSession *currSession, const char *data, huint32 len, void *userData) {
    printf("%s\n", data);
}

static void DebugLog(HSession *currSession, const char *log, huint32 len, void *userData) {
    printf("%s\n", log);
    fflush(stdout);
}

static void NetStatus(HSession *currSession, eNetStatus status, void *userData) {
    switch (status) {
    case kConnect:
        printf("connect\n");
        break;
    case kDisconnect:
        printf("disonnect\n");
        break;
    default:
        break;
    }
    fflush(stdout);
}


int main()
{
    // 创建一个单例的事件循环
    core = CreateEventCore();

    // 创建SDK网络会话
    HSession *net = CreateNetSession(core, kSDK2);

    // 设置网络会话的相关回调
    SetNetSession(net, kReadyReadFunc, reinterpret_cast<void *>(ReadData));
    SetNetSession(net, kReadyReadUserData, net);
    SetNetSession(net, kNetStatusFunc, reinterpret_cast<void *>(NetStatus));
    SetNetSession(net, kNetStatusUserData, net);
    SetNetSession(net, kDebugLogFunc, reinterpret_cast<void *>(DebugLog));


    // 可以使用下面Run, 内部自行开辟一个事件线程运行事件循环, 不阻塞当前进程
    // 但要注意, 回调事件都是在事件线程, 因为是内部开辟的线程, 所以回调时如果需要存储数据则需要注意多线程安全.
    // 网络会话读写是多线程安全的
    Run(core);

    bool first = true;
    for (;;) {
        if (first == false) {
            std::cout << std::endl;
        }
        first = false;
        std::string ip;
        std::cout << "Enter ip: ";
        std::cin >> ip;

        // 连接SDK
        if (Connect(net, ip.data(), 10001) == HFalse) {
            continue;
        }

        // 发送xml数据
        std::string data(GETDEVICEINFO_TEXT);
        SendSDK(net, data.data(), data.size());

        data.clear();
        for (;;) {
            std::string input;
            std::getline(std::cin, input);
            if (Isconnect(net) == false) {
                std::cout << "is disconnect" << std::endl;
                break;
            }

            if (input == "close") {
                Disconnect(net);
                break;
            } else if (input == "send") {
                std::cout << data << std::endl;
                SendSDK(net, data.data(), data.size());
                data.clear();
            } else if (input == "sendFile") {
                // 发送类型设置为0, 图片类型, 具体详情请看文档
                SendFile(net, data.data(), 0);
                std::string getFileInfo(GETFILEINFO_TEXT);
                SendSDK(net, getFileInfo.data(), getFileInfo.size());
            } else if (input == "clear") {
                data.clear();
            } else {
                data.append(input);
            }
            std::cin.clear();
        }
    }

    // 释放会话
    FreeNetSession(net);

    // 释放事件核心
    FreeEventCore(core);
    return 0;
}
