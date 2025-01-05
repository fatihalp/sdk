

#ifndef __HSDKINFO_H__
#define __HSDKINFO_H__


#include <Data/HEthernetInfo.h>
#include <Data/HGeneralInfo.h>
#include <Data/HLightInfo.h>
#include <Data/HOTherInfo.h>
#include <Data/HScreenFunctionInfo.h>
#include <Data/HSensorInfo.h>
#include <Data/HTimeInfo.h>
#include <tinyxml2.h>
#include <memory>
#include <mutex>


namespace sdk
{


struct HSDKInfo
{
    Eth0Info ethInfo;                       ///< 有线
    WifiInfo wifiInfo;                      ///< wifi
    PppoeInfo pppoeInfo;                    ///< pppoe
    SystemVolumeInfo systemVolumeInfo;      ///< 系统音量
    DeviceNameInfo deviceNameInfo;          ///< 设备名
    SDKTcpServerInfo tcpSercerInfo;         ///< tcp服务器
    LightInfo lightInfo;                    ///< 亮度
    MulScreenSyncInfo mulScreenSyncInfo;    ///< 多屏同步
    GpsRespondInfo gpsResondInfo;           ///< gps数据上报
    SwitchTimeInfo switchTimeInfo;          ///< 开关机
    RelayInfo relayInfo;                    ///< 继电器
    TimeInfo timeInfo;                      ///< 时间
    SwitchProgramInfo switchProgramInfo;    ///< 切换节目和获取当前节目的guid

    ///< 下面是只有获取项的
    DeviceInfo deviceInfo;                  ///< 设备信息
    ScreenShot2 screenShot2;                ///< 截图数据
    SensorInfo sensorInfo;                  ///< 传感器状态
    GpsInfo gpsInfo;                        ///< gps信息


    int ParseInfo(tinyxml2::XMLElement *outNode, const cat::HCatBuffer &method);
};


}

#endif // __HSDKINFO_H__
