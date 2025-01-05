

#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "HDSDK.h"


using namespace std;

int IsRead = 0;


static bool SendData(const char *data, int len, void *userData) {
    int fd = (int)(intptr_t)userData;
    return write(fd, data, len) > 0;
}

static void ReadXml(const char *xml, int len, int errorCode, void *userData) {
    (void)len;
    (void)userData;
    ++IsRead;
    if (errorCode != 0) {
        printf("error code[%d]\n", errorCode);
        return ;
    }
    printf("====[%s]===\n", xml);
}

static int SendPngFile(hint64 index, hint64 size, int status, char *buff, int buffSize, void *userData) {
    if (status != 0) {
        printf("send file faild[%d]\n", status);
        return 0;
    }

    printf("[%lld][%lld][%f]\r", index, size, 1.0f * index / size);
    if (index == size) {
        ++IsRead;
        printf("\n");
        return 0;
    }

    int fd = (int)(intptr_t)userData;
    if (lseek(fd, 0, SEEK_CUR) != index) {
        lseek(fd, index, SEEK_SET);
    }
    int len = read(fd, buff, buffSize);
    return len;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("a.out ip\n");
        return 0;
    }

    int fileFd = open("./1.png", O_RDONLY);
    if (fileFd == -1) {
        perror("not open file[1.png]");
        return 1;
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

    IHDProtocol proto = CreateProtocol();
    SetProtocolFunc(proto, kSetSendFunc, (void *)SendData);
    SetProtocolFunc(proto, kSetSendFuncData, (void *)(intptr_t)(fd));
    SetProtocolFunc(proto, kSetReadXml, (void *)ReadXml);

    RunProtocol(proto);

    const char *xml = R"(<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetDeviceInfo"/>
</sdk>)";

    const char *getFiles = R"(<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetFiles"/>
</sdk>
)";

    SendXml(proto, xml, strlen(xml));
    const char *fileName = "test.png";
    SendFile(proto, fileName, strlen(fileName), "6070a9ea174fdae68e8fe89bc5ec7403", 0, 21060215, SendPngFile, (void *)(intptr_t)fileFd);
    SendXml(proto, getFiles, strlen(getFiles));

    char buff[1024];
    for (;;) {
        int len = read(fd, buff, sizeof(buff));
        if (len <= 0) {
            break;
        }

        UpdateReadData(proto, buff, len);
        if (IsRead > 4) {
            break;
        }
    }

    FreeProtocol(proto);
    close(fileFd);
    close(fd);
    return 0;
}
