#include <iostream>
#include <string>
#include <thread>

#include "../include/ICApi.h"

#ifdef HWIN64
#pragma comment(lib,"../lib64/HCatNet.lib")
#elif HWIN32
#pragma comment(lib,"../lib/HCatNet.lib")
#endif

static HEventCore *core = nullptr;


static void DeviceInfo(HSession *currSession, const char *id, huint32 idLen, const char *ip, huint32 ipLen, const char *readData, huint32 dataLen, void *userData) {
    printf("id[%s], ip[%s]\n", id, ip);
}


int main()
{
    // 创建一个单例的事件循环
    core = CreateEventCore();

    // 创建一个探测设备id ip的会话.
    HSession *findDevice = CreateNetSession(core, kDetectDevice);
    SetNetSession(findDevice, kDeviceInfoFunc, DeviceInfo);

    // 探测会话只要执行连接即可, 参数无用
    Connect(findDevice, "", 0);

    // 每次调用发送数据都会发送一次探测数据
    SendSDK(findDevice, "", 0);

    Run(core);

    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        SendSDK(findDevice, "", 0);
    }

    FreeNetSession(findDevice);
    FreeEventCore(core);
    return 0;
}