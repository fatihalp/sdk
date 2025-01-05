

#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

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

    char **p = (char **)(userData);
    free(*p);
    *p = (char *)malloc(len);
    memcpy(*p, xml, len);
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

    char *p_buff = NULL;

    IHDProtocol proto = CreateProtocol();
    SetProtocolFunc(proto, kSetSendFunc, (void *)SendData);
    SetProtocolFunc(proto, kSetSendFuncData, (void *)(intptr_t)(fd));
    SetProtocolFunc(proto, kSetReadXml, (void *)ReadXml);
    SetProtocolFunc(proto, kSetReadXmlData, &p_buff);

    RunProtocol(proto);

    const char *xml = R"(<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetDeviceInfo"/>
</sdk>)";

    SendXml(proto, xml, strlen(xml));
    SendXml(proto, xml, strlen(xml));

    char buff[1024];
    for (;;) {
        int len = read(fd, buff, sizeof(buff));
        if (len <= 0) {
            break;
        }

        UpdateReadData(proto, buff, len);
        if (IsRead > 1) {
            break;
        }
    }

    printf("read xml[\n%s]\n", p_buff);
    free(p_buff);
    FreeProtocol(proto);
    close(fd);
    return 0;
}
