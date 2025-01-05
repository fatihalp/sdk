

#ifndef __ISDKINFO_H__
#define __ISDKINFO_H__


#include <HDSDK.h>

#ifdef USE_HD_LIB
namespace sdk{
typedef struct HSDKInfo* ISDKInfo;
}
using sdk::ISDKInfo;
#else
typedef void* ISDKInfo;
#endif

#ifdef __cplusplus
extern "C" {
#endif

///< 创建信息体
HD_API ISDKInfo DLL_CALL CreateSDKInfo();
///< 释放信息体
HD_API void DLL_CALL FreeSDKInfo(ISDKInfo info);
///< 解析xml生成对应数据信息
HD_API HBool DLL_CALL ParseXml(ISDKInfo info, const char *xml, int len);

enum eUpdateItem {
    kGetLightInfo           = 0x1000,   ///< 获取亮度信息
    kSetLightInfo           = 0x1001,   ///< 设置亮度信息
    kGetSystemVolumeInfo    = 0x1002,   ///< 获取系统音量
    kSetSystemVolumeInfo    = 0x1003,   ///< 设置系统音量
    kGetTcpServerInfo       = 0x1004,   ///< 获取tcp服务器
    kSetTcpServerInfo       = 0x1005,   ///< 设置tcp服务器
    kGetTimeInfo            = 0x1006,   ///< 获取时间信息
    kSetTimeInfo            = 0x1007,   ///< 设置时间信息
    kGetEthInfo             = 0x1008,   ///< 获取有线信息
    kSetEthInfo             = 0x1009,   ///< 设置有线信息
    kGetWifiInfo            = 0x1010,   ///< 获取Wifi信息
    kSetWifiInfo            = 0x1011,   ///< 设置Wifi信息
    kGetPppoeInfo           = 0x1012,   ///< 获取pppoe信息
    kSetPppoeInfo           = 0x1013,   ///< 设置pppoe信息
    kGetDeviceNameInfo      = 0x1014,   ///< 获取设备名信息
    kSetDeviceNameInfo      = 0x1015,   ///< 设置设备名信息
    kGetSwitchTimeInfo      = 0x1016,   ///< 获取开关机信息
    kSetSwitchTimeInfo      = 0x1017,   ///< 设置开关机信息
    kGetRelayInfo           = 0x1018,   ///< 获取继电器信息
    kSetRelayInfo           = 0x1019,   ///< 设置继电器信息
    kGetCurrProgramGuid     = 0x1020,   ///< 获取当前节目guid
    kSetCurrProgramGuid     = 0x1021,   ///< 设置当前节目guid

    ///< 没有设置项, 只有获取项
    kGetDeviceInfo          = 0x2000,   ///< 获取设备信息
    kGetScreenShot          = 0x2001,   ///< 获取截图数据
};
///< 更新对应项, 需要参数会话, 信息体, 对应项
HD_API HBool DLL_CALL UpdateItem(IHDProtocol protocol, ISDKInfo info, int updateItem);

/**
 * @brief SetLightInfo 设置亮度信息
 * @param info 信息体
 * @param mode 亮度模式 {0(默认), 1(自定义模式), 2(传感器模式)}
 * @param defaultModeLight 默认模式下的亮度
 */
HD_API void DLL_CALL SetLightInfo(ISDKInfo info, int mode, int defaultModeLight);
HD_API void DLL_CALL SetLightInfoSensor(ISDKInfo info, int min, int max, int time);
// startTime格式 HH:mm:ss
HD_API void DLL_CALL AddLightInfoPloy(ISDKInfo info, HBool enable, const char *startTime, int percent);
HD_API void DLL_CALL SetLightInfoPloy(ISDKInfo info, int index, HBool enable, const char *startTime, int percent);
HD_API void DLL_CALL ClearLightInfoPloy(ISDKInfo info);

///< 亮度获取系列
HD_API int DLL_CALL GetLightInfoMode(ISDKInfo info);
HD_API int DLL_CALL GetLightInfoDefaultLight(ISDKInfo info);
HD_API int DLL_CALL GetLightInfoPloySize(ISDKInfo info);
HD_API int DLL_CALL GetLightInfoPloyEnable(ISDKInfo info, int index);
HD_API int DLL_CALL GetLightInfoPloyPercent(ISDKInfo info, int index);
HD_API const char * DLL_CALL GetLightInfoPloyStart(ISDKInfo info, int index);
HD_API int DLL_CALL GetLightInfoSensorMax(ISDKInfo info);
HD_API int DLL_CALL GetLightInfoSensorMin(ISDKInfo info);
HD_API int DLL_CALL GetLightInfoSensorTime(ISDKInfo info);

/**
 * @brief SetSystemVolume 设置系统音量
 * @param info 信息体
 * @param mode 音量模式{0(默认), 1(分时)}
 * @param volume 默认模式下的音量
 */
HD_API void DLL_CALL SetSystemVolumeInfo(ISDKInfo info, int mode, int volume);
// time格式HH:mm:ss
HD_API void DLL_CALL AddSystemVolumeInfoPloy(ISDKInfo info, HBool enable, const char *time, int volume);
HD_API void DLL_CALL ClearSystemVolumeInfoPloy(ISDKInfo info);

///< 系统音量获取系列
HD_API int DLL_CALL GetSystemVolumeInfoMode(ISDKInfo info);
HD_API int DLL_CALL GetSystemVolumeInfoVolume(ISDKInfo info);
HD_API int DLL_CALL GetSystemVolumeInfoPloySize(ISDKInfo info);
HD_API HBool DLL_CALL GetSystemVolumeInfoPloyEnable(ISDKInfo info, int index);
HD_API const char * DLL_CALL GetSystemVolumeInfoPloyTime(ISDKInfo info, int index);
HD_API int DLL_CALL GetSystemVolumeInfoPloyVolume(ISDKInfo info, int index);

/**
 * @brief SetTcpServerInfo 设置tcp服务器
 * @param info 信息体
 * @param ip 服务器地址
 * @param port 端口
 */
HD_API void DLL_CALL SetTcpServerInfo(ISDKInfo info, const char *ip, huint16 port);

///< tcp服务器获取系列
HD_API const char * DLL_CALL GetTcpServerInfoIp(ISDKInfo info);
HD_API huint16 DLL_CALL GetTcpServerInfoPort(ISDKInfo info);


/**
 * @brief SetTimeInfo 设置时间
 * @param info 信息体
 * @param timeZone 时区, 格式"(UTC+08:00)Beijing,Chongqing,HongKong,Urumchi"
 * @param summer 是否开启夏令时
 * @param sync 是否开启时间同步 {"none"(不开启自动同步), "gps"(gps校时), "network"(网络校时), "auto"(自动校时)}
 * @param currTime 时间 格式"yyyy-MM-dd hh:mm:ss"
 * @param ntp服务器列表, 以逗号分隔
 */
HD_API void DLL_CALL SetTimeInfo(ISDKInfo info, const char *timeZone, HBool summer, const char *sync, const char *currTime, const char *serverList);

///< 时间信息获取系列
HD_API const char * DLL_CALL GetTimeInfoTimeZone(ISDKInfo info);
HD_API HBool DLL_CALL GetTimeInfoSummer(ISDKInfo info);
HD_API const char * DLL_CALL GetTimeInfoSync(ISDKInfo info);
HD_API const char * DLL_CALL GetTimeInfoCurrTime(ISDKInfo info);
HD_API const char * DLL_CALL GetTimeInfoServerList(ISDKInfo info);


/**
 * @brief SetEthInfo 设置以太网信息
 * @param info 信息体
 * @param dhcp dhcp
 * @param ip ip
 * @param netmask 网络掩码
 * @param gateway 网关
 * @param dns dns
 */
HD_API void DLL_CALL SetEthInfo(ISDKInfo info, HBool dhcp, const char *ip, const char *netmask, const char *gateway, const char *dns);

///< 以太网信息获取系列
HD_API HBool DLL_CALL GetEhtInfoDhcp(ISDKInfo info);
HD_API const char * DLL_CALL GetEhtInfoIp(ISDKInfo info);
HD_API const char * DLL_CALL GetEhtInfoNetmask(ISDKInfo info);
HD_API const char * DLL_CALL GetEhtInfoGateway(ISDKInfo info);
HD_API const char * DLL_CALL GetEhtInfoDns(ISDKInfo info);


/**
 * @brief SetWifiInfo 设置wifi信息
 * @param info 信息体
 * @param mode {0: ap模式, 1: station模式}
 */
HD_API void DLL_CALL SetWifiInfo(ISDKInfo info, int mode);
HD_API void DLL_CALL SetWifiInfoAp(ISDKInfo info, const char *ssid, const char *password, const char *ip);
HD_API void DLL_CALL SetWifiInfoStation(ISDKInfo info, const char *ssid, const char *password, HBool dhcp);
HD_API void DLL_CALL SetWifiInfoStationNet(ISDKInfo info, const char *ip, const char *mask, const char *gateway, const char *dns);

HD_API int DLL_CALL GetWifiInfoMode(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoApSsid(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoApPassword(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoApIp(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoStationSsid(ISDKInfo info);
HD_API HBool DLL_CALL GetWifiInfoStationDhcp(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoStationIp(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoStationMask(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoStationGateway(ISDKInfo info);
HD_API const char * DLL_CALL GetWifiInfoStationDns(ISDKInfo info);


/**
 * @brief SetPppoeInfoApn 设置pppoe apn
 * @param info 信息体
 * @param apn apn
 */
HD_API void DLL_CALL SetPppoeInfoApn(ISDKInfo info, const char *apn);

HD_API HBool DLL_CALL GetPppoeInfoVaild(ISDKInfo info);
HD_API const char * DLL_CALL GetPppoeInfoApn(ISDKInfo info);


/**
 * @brief SetDeviceNameInfo 设置设备名
 * @param info 信息体
 * @param name 设备名
 **/
HD_API void DLL_CALL SetDeviceNameInfo(ISDKInfo info, const char *name);
HD_API const char * DLL_CALL GetDeviceNameInfo(ISDKInfo info);


/**
 * @brief SetSwitchTimeInfo 设置定时开关机
 * @param info 信息体
 * @param name 模式
 * @param enable 是否开启定时开关机
 **/
HD_API void DLL_CALL SetSwitchTimeInfo(ISDKInfo info, int mode, HBool enable);
// start和end 格式 hh:mm:ss
HD_API void DLL_CALL AddSwitchTimeInfoItem(ISDKInfo info, HBool enable, const char *start, const char *end);
HD_API void DLL_CALL ClearSwitchTimeInfoItem(ISDKInfo info);
HD_API HBool DLL_CALL SetSwitchTimeInfoItem(ISDKInfo info, int index, HBool enable, const char *start, const char *end);
// start和end 格式 hh:mm:ss
HD_API void DLL_CALL AddSwitchTimeInfoWeekItem(ISDKInfo info, int week, HBool openAllDay, const char *start, const char *end);
HD_API void DLL_CALL ClearSwitchTimeInfoWeekItem(ISDKInfo info, int week);
HD_API void DLL_CALL SetSwitchTimeInfoWeekItem(ISDKInfo info, int week, int index, HBool openAllDay, const char *start, const char *end);

HD_API int DLL_CALL GetSwitchTimeInfoItemSize(ISDKInfo info);
HD_API int DLL_CALL GetSwitchTimeInfoWeekItemSize(ISDKInfo info, int week);
HD_API HBool DLL_CALL GetSwitchTimeInfoEnable(ISDKInfo info);
HD_API HBool DLL_CALL GetSwitchTimeInfoItemEnable(ISDKInfo info, int index);
HD_API const char * DLL_CALL GetSwitchTimeInfoItemStart(ISDKInfo info, int index);
HD_API const char * DLL_CALL GetSwitchTimeInfoItemEnd(ISDKInfo info, int index);


/**
 * @brief SetRelayInfoItem 设置继电器项
 * @param info 信息体
 * @param index 继电器索引
 * @param name 继电器名
 * @param useSwitch 关联显示屏状态
 **/
HD_API void DLL_CALL SetRelayInfoItem(ISDKInfo info, int index, const char *name, HBool useSwitch);
HD_API void DLL_CALL AddRelayInfoItemPloy(ISDKInfo info, int index, const char *start, const char *end);
HD_API void DLL_CALL ClearRelayInfoItemPloy(ISDKInfo info, int index);
HD_API void DLL_CALL SetRelayInfoItemPloyItem(ISDKInfo info, int index, int itemIndex, const char *start, const char *end);
HD_API void DLL_CALL SetRelayInfoInternal(ISDKInfo info, const char *name, HBool useSwitch);
HD_API void DLL_CALL AddRelayInfoInternalPloy(ISDKInfo info, const char *start, const char *end);
HD_API void DLL_CALL ClearRelayInfoInternalPloy(ISDKInfo info);
HD_API void DLL_CALL SetRelayInfoInternalPloyItem(ISDKInfo info, int index, const char *start, const char *end);

HD_API int DLL_CALL GetRelayInfoStatus(ISDKInfo info, int index);
HD_API const char * DLL_CALL GetRelayInfoName(ISDKInfo info, int index);
HD_API HBool DLL_CALL GetRelayInfoUseSwitch(ISDKInfo info, int index);
HD_API int DLL_CALL GetRelayInfoItemPloySize(ISDKInfo info, int index);
HD_API const char * DLL_CALL GetRelayInfoItemPloyStart(ISDKInfo info, int index, int itemIndex);
HD_API const char * DLL_CALL GetRelayInfoItemPloyEnd(ISDKInfo info, int index, int itemIndex);
HD_API int DLL_CALL GetRelayInfoInternalPloySize(ISDKInfo info);
HD_API const char * DLL_CALL GetRelayInfoInternalPloyStart(ISDKInfo info, int index);
HD_API const char * DLL_CALL GetRelayInfoInternalPloyEnd(ISDKInfo info, int index);


/**
 * @brief SetCurrProgramGuid 设置当前节目的guid
 * @param info 信息体
 * @param guid 节目guid
 * @param index 节目索引(和节目guid互斥)
 */
HD_API void DLL_CALL SetCurrProgramGuid(ISDKInfo info, const char *guid, int index);

HD_API const char * DLL_CALL GetCurrProgramGuid(ISDKInfo info);


///< 下面是获取

///< 获取设备信息系列
HD_API const char * DLL_CALL GetDevceInfoId(ISDKInfo info);
HD_API const char * DLL_CALL GetDevceInfoName(ISDKInfo info);
HD_API const char * DLL_CALL GetDevceInfoAppVersion(ISDKInfo info);
HD_API const char * DLL_CALL GetDevceInfoFpgaVersion(ISDKInfo info);
HD_API int DLL_CALL GetDevceInfoScreenRotation(ISDKInfo info);
HD_API int DLL_CALL GetDevceInfoScreenWidth(ISDKInfo info);
HD_API int DLL_CALL GetDevceInfoScreenHeight(ISDKInfo info);

///< 获取屏幕截图系列
HD_API void DLL_CALL SetScreenShot(ISDKInfo info, int width, int height);
HD_API const char * DLL_CALL GetScreenShot(ISDKInfo info);
HD_API int DLL_CALL GetScreenShotSize(ISDKInfo info);


#ifdef __cplusplus
}
#endif

#endif // ISDKINFO_H
