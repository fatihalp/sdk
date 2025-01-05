#include <iostream>
#include <string>

#include "../include/ICApi.h"

#ifdef HWIN64
#pragma comment(lib,"../lib/HCatNet.lib")
#elif HWIN32
#pragma comment(lib,"../lib/HCatNet.lib")
#endif


static HEventCore *core = nullptr;


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

static void NewClient(HSession *currSession, HSession *newSession, void *userData) {
    printf("currSession[%p], newSession[%p], userData[%p]\n", currSession, newSession, userData);
    fflush(stdout);
    // 这里有设备连接到这里, 需要设置新连接的回调
    SetNetSession(newSession, kReadyReadFunc, reinterpret_cast<void *>(ReadData));
    SetNetSession(newSession, kNetStatusFunc, reinterpret_cast<void *>(NetStatus));
    SetNetSession(newSession, kDebugLogFunc, reinterpret_cast<void *>(DebugLog));

    // 不用了就关闭新连接
    FreeNetSession(newSession);
}

int main()
{
    // 创建一个单例的事件循环
    core = CreateEventCore();

    // 创建一个SDK服务端会话
    HSession *service = CreateNetSession(core, kSDK2Service);
    // 绑定新连接回调
    SetNetSession(service, kNewConnectFunc, reinterpret_cast<void *>(NewClient));
    // 开始侦听10001端口
    Connect(service, "", 10001);

    return Exec(core);
}