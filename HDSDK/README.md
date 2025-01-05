# README

## SDK协议接口详解  HDSDK.h

> 详情请看 HDSDK.h 头文件

1. SDK协议封装了协议部分, 网络部分由用户自己实现, 只需给协议提供发送数据的回调和读取xml数据的回调接口即可. 接下来只需要将网络读取的数据传入协议即可.

```cpp
static int sendDataToNet(const char *data, int len, void *userData)
{
    int len = write(fd, data, len);
    return len > 0 ? 1 : 0;
}

// 创建协议通信
IHDProtocol sdk = CreateProtocol();

// 设置协议
// kSetReadXml = 0x0001
// kSetReadXmlData = 0x0002
// void readXml(const char *xml, int len, int errorCode, void *userData)
SetProtocolFunc(sdk, kSetReadXml, func);

// 设置这个回调, 这个回调就是需要write的发送数据的地方
// kSetSendFunc = 0x0003
// kSetSendFuncData = 0x0004
// int sendDataToNet(const char *data, int len, void *userData), 返回0或1, 1成功 0失败
SetProtocolFunc(sdk, kSetSendFunc, sendDataToNet);

// 开始执行协议协商
RunProtocol(sdk);

// 发送xml数据通信sdk
SendXml(sdk, xml, xmlLen);

for (;;) {
    // 将所有读取到的数据都推入UpdateReadData
    data = read();
    UpdateReadData(sdk, data, data.size());
}

// 释放协议体
FreeProtocol(sdk);
```

## 使用SDK信息体构建xml SDKInfo.h

> 详情请看 SDKInfo.h 头文件

```cpp
static void readXml(const char *xml, int len, int errorCode, void *userData)
{
    if (errorCode != 0) {
        printf("error code[%d]\n", errorCode);
        return ;
    }
    
    // 解析xml
    ISDKInfo info = (ISDKInfo)userData;
    ParseXml(info, xml, len);
    
    printf("模式[%d]\n", GetLightInfoMode(info));
    printf("亮度值[%d]\n", GetLightInfoDefaultLight(info));
}

// 构建xml信息体
ISDKInfo sdkInfo = CreateSDKInfo();

// 创建协议通信
IHDProtocol sdk = CreateProtocol();

// 设置协议
// 设置读取xml的数据的回调
// kSetReadXml = 0x0001
// kSetReadXmlData = 0x0002
// void readXml(const char *xml, int len, int errorCode, void *userData)
SetProtocolFunc(sdk, kSetReadXml, readXml);
SetProtocolFunc(sdk, kSetReadXmlData, sdkInfo);

// 设置这个回调, 这个回调就是需要write的发送数据的地方
// kSetSendFunc = 0x0003
// kSetSendFuncData = 0x0004
// int sendDataToNet(const char *data, int len, void *userData), 返回0或1, 1成功 0失败
SetProtocolFunc(sdk, kSetSendFunc, sendDataToNet);

// 开始执行协议协商
RunProtocol(sdk);

// 发送获取亮度请求
// kGetLightInfo = 0x1000
UpdateItem(sdk, sdkInfo, kGetLightInfo);

// 设置亮度信息体, 模式, 亮度值
SetLightInfo(sdkInfo, 0, 100);

// 发送设置亮度请求
// kSetLightInfo = 0x1001
UpdateItem(sdk, sdkInfo, kSetLightInfo);

for (;;) {
    // 将所有读取到的数据都推入UpdateReadData
    data = read();
    UpdateReadData(sdk, data, data.size());
}

FreeSDKInfo(sdkInfo);
FreeProtocol(sdk);
```

## Linux 编译方法

```shell
cd HDSDK
mkdir build
cd build
cmake ..
make -j16

# 已经编译完成了, 在Protocol有libHDSDK.so
cd Protocol
```

## Windows编译方法

直接IDE打开CMakeLists.txt文件, 选好编译器编译就行了
