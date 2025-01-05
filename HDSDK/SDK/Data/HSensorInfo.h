

#ifndef __HSENSORINFO_H__
#define __HSENSORINFO_H__


#include <HCatBuffer.h>
#include <HXml.h>
#include <vector>


namespace sdk
{


///< 传感器状态
struct SensorInfo
{
    bool luminance;         ///< 是否接入亮度传感器
    bool temp1;             ///< 是否接入温度传感器1
    bool humidity;          ///< 是否接入湿度传感器
    bool temp2;             ///< 是否接入温度传感器2
    bool telecontroller;    ///< 是否接入遥控器
    bool gps;               ///< 是否接入gps
    bool windSpeed;         ///< 是否接入风速传感器
    bool windDirection;     ///< 是否接入风向传感器
    bool noise;             ///< 是否接入噪音传感器
    bool pressure;          ///< 是否接入压力传感器
    bool lightIntensity;    ///< 是否接入光照强度传感器
    bool rainfall;          ///< 是否接入降雨量传感器
    bool co2;               ///< 是否接入二氧化碳传感器
    bool pm2d5;             ///< 是否接入PM2.5传感器
    bool pm10;              ///< 是否接入PM10传感器

    SensorInfo()
        : luminance(false)
        , temp1(false)
        , humidity(false)
        , temp2(false)
        , telecontroller(false)
        , gps(false)
        , windSpeed(false)
        , windDirection(false)
        , noise(false)
        , pressure(false)
        , lightIntensity(false)
        , rainfall(false)
        , co2(false)
        , pm2d5(false)
        , pm10(false)
    {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetSensorInfo"; }
    static cat::HCatBuffer GetMethod() { return "GetSensorInfo"; }
};
void from_xml(const HXml &xml, SensorInfo &node);


///< gps
struct GpsInfo
{
    bool east;          ///< 东方
    bool north;         ///< 北方
    double longitude;   ///< 经度
    double latitude;    ///< 纬度
    int counts;         ///< 计数
    float speed;        ///< 速度
    float direction;    ///< 方向

    GpsInfo()
        : east(false)
        , north(false)
        , longitude(0.0)
        , latitude(0.0)
        , counts(0)
        , speed(0.0)
        , direction(0.0)
    {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetGPSInfo"; }
    static cat::HCatBuffer GetMethod() { return "GetGPSInfo"; }
};
void from_xml(const HXml &xml, GpsInfo &node);


///< 继电器
struct RelayInfo
{
    struct ployItem {
        cat::HCatBuffer start;  ///< 格式 hh:mm:ss
        cat::HCatBuffer end;    ///< 格式 hh:mm:ss
    };

    struct realyItem {
        int relayStatus;                ///< 继电器状态{0: 未知 1:继电器开启 2:继电器关闭}
        cat::HCatBuffer name;           ///< 继电器名称
        bool useSwitch;                 ///< 关联显示屏状态 {1:true 0:false}
        std::vector<ployItem> ploys;    ///< 时间项
        realyItem() : relayStatus(0), useSwitch(false) {}
    };

    std::vector<realyItem> relayList;   ///< 开关配置列表
    realyItem internal;                 ///< 板载继电器

    void InitRelayList();

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetRelayInfo"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetRelayInfo"; }
    static cat::HCatBuffer GetMethod() { return "GetRelayInfo"; }
    static cat::HCatBuffer SetMethod() { return "SetRelayInfo"; }
};
void from_xml(const HXml &xml, RelayInfo &node);
void to_xml(HXml &xml, const RelayInfo &node);


}


#endif // HSENSORINFO_H
