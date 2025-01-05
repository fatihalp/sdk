

#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include "../SDK/SDKInfo.h"


using namespace std;


struct Proto
{
    IHDProtocol proto;
    ISDKInfo info;
};

int IsRead = 0;


static bool SendData(const char *data, int len, void *userData) {
    int fd = (int)(intptr_t)userData;
    return write(fd, data, len) > 0;
}

static void ReadXml(const char *xml, int len, int errorCode, void *userData) {
    Proto *info = (Proto *)userData;
    ++IsRead;
    if (errorCode != 0) {
        printf("error code[%d]\n", errorCode);
        return ;
    }
    ParseXml(info->info, xml, len);

    std::cout << "mode:" << GetSystemVolumeInfoMode(info->info) << "\n"
              << "volume:" << GetSystemVolumeInfoVolume(info->info) << "\n"
              << "ploySize:" << GetSystemVolumeInfoPloySize(info->info) << "\n"
              << "ployEnable:" << GetSystemVolumeInfoPloyEnable(info->info, 0) << "\n"
              << "ployTime:" << GetSystemVolumeInfoPloyTime(info->info, 0) << "\n"
              << "ployVolume:" << GetSystemVolumeInfoPloyVolume(info->info, 0) << "\n"
              << std::endl;

    if (IsRead == 1) {
        SetSystemVolumeInfo(info->info, 0, 100);
        AddSystemVolumeInfoPloy(info->info, HFalse, "11:11:11", 100);
        ClearSystemVolumeInfoPloy(info->info);
        AddSystemVolumeInfoPloy(info->info, HFalse, "12:12:12", 100);

        UpdateItem(info->proto, info->info, kSetSystemVolumeInfo);
    } else {
        UpdateItem(info->proto, info->info, kGetSystemVolumeInfo);
    }
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("a.out ip\n");
        return 0;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("create socket faild");
        return 1;
    }

    struct sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(10001);
    addrSrv.sin_addr.s_addr = inet_addr(argv[1]);

    int result = connect(fd, reinterpret_cast<struct sockaddr *>(&addrSrv), sizeof(addrSrv));
    if (result < 0) {
        close(fd);
        perror("Not connect client");
        return 2;
    }

    ISDKInfo sdkInfo = CreateSDKInfo();
    IHDProtocol proto = CreateProtocol();
    Proto a;
    a.proto = proto;
    a.info = sdkInfo;
    SetProtocolFunc(proto, kSetSendFunc, (void *)SendData);
    SetProtocolFunc(proto, kSetSendFuncData, (void *)(intptr_t)(fd));
    SetProtocolFunc(proto, kSetReadXml, (void *)ReadXml);
    SetProtocolFunc(proto, kSetReadXmlData, (void *)&a);

    RunProtocol(proto);

    UpdateItem(proto, sdkInfo, kGetSystemVolumeInfo);

    char buff[1024];
    for (;;) {
        int len = read(fd, buff, sizeof(buff));
        if (len <= 0) {
            break;
        }

        UpdateReadData(proto, buff, len);
        if (IsRead > 2) {
            break;
        }
    }

    FreeSDKInfo(sdkInfo);
    FreeProtocol(proto);
    close(fd);
    return 0;
}
