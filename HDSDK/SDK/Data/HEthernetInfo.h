

#ifndef __HETHERNETINFO_H__
#define __HETHERNETINFO_H__


#include <HCatBuffer.h>
#include <HXml.h>


namespace sdk
{


///< Eth0
struct Eth0Info
{
    bool dhcp;  ///< "true"(dhcp获取ip地址), "false"(静态ip地址)
    cat::HCatBuffer ip;
    cat::HCatBuffer netmask;
    cat::HCatBuffer gateway;
    cat::HCatBuffer dns;

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetEth0Info"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetEth0Info"; }
    static cat::HCatBuffer GetMethod() { return "GetEth0Info"; }
    static cat::HCatBuffer SetMethod() { return "SetEth0Info"; }
};
void from_xml(const HXml &xml, Eth0Info &node);
void to_xml(HXml &xml, const Eth0Info &node);


struct WifiInfo
{
    struct ApConfig
    {
        cat::HCatBuffer ssid;       ///< 热点名称
        cat::HCatBuffer password;   ///< 热点密码
        cat::HCatBuffer ipAddress;  ///< ip地址
        int channel;                ///< ap通道
        ApConfig() : channel(0) { }
    };

    struct StationConfig
    {
        cat::HCatBuffer ssid;       ///< wifi名称
        cat::HCatBuffer password;   ///< wifi密码(仅设置)
        bool dhcp;                  ///< 是否开启dhcp(默认开)
        cat::HCatBuffer mac;        ///< mac地址(只读)
        cat::HCatBuffer ip;
        cat::HCatBuffer mask;
        cat::HCatBuffer gateway;
        cat::HCatBuffer dns;
        StationConfig() : dhcp(true) {}
    };

    cat::HCatBuffer mode;               ///< 模式(ap|station)
    ApConfig ap;                        ///< ap设置
    int stationIndex;                   ///< station索引
    std::list<StationConfig> station;   ///< station设置

    WifiInfo() : stationIndex(0) {}

    const StationConfig & GetCurrentStation();

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetWifiInfo"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetWifiInfo"; }
    static cat::HCatBuffer GetMethod() { return "GetWifiInfo"; }
    static cat::HCatBuffer SetMethod() { return "SetWifiInfo"; }
};
void from_xml(const HXml &xml, WifiInfo &node);
void to_xml(HXml &xml, const WifiInfo &node);


struct PppoeInfo
{
    bool vaild;                     ///< 是否有接入 {"true"(有3/4G模块接入), "false"(无3/4G模块接入)}
    bool enable;                    ///< {"true"(有3/4G网络接入), "false"(无3/4G网络接入)}
    cat::HCatBuffer apn;            ///< apn值
    cat::HCatBuffer manufacturer;   ///< 模块生产商
    cat::HCatBuffer version;        ///< 模块版本
    cat::HCatBuffer model;          ///< 模块型号
    cat::HCatBuffer imei;           ///< 模块IMEI
    cat::HCatBuffer number;         ///< sim卡电话号码
    cat::HCatBuffer operators;      ///< 运营商
    int signal;                     ///< 信号强度, 取值范围[1, 5]; 1表示信号强度最差; 5表示信号强度最好
    int dbm;                        ///< 信号强度(单位dbm)
    bool insert;                    ///< SIM卡是否插入, 取值范围{"true"(有SIM卡插入), "false"(无SIM卡插入)}
    cat::HCatBuffer status;         ///< 网络注册状态, 取值范围 {"unregister"(未注册), "register local"(已注册, 本地网络), "searching"(搜索中), "reject"(拒绝注册), "unknow"(未知错误), "register roaming"(已注册, 漫游网络), "init"(初始化状态)}
    cat::HCatBuffer network;        ///< 网络制式, 取值范围 {"init"(初始化状态), "unknow"(未知网络), "2G"(2G), "2.5G"(2.5G), "3GPP"(3GPP家族), "3G TD"(移动3G), "3.5G HSDPA", "3.5G HSUPA", "3.5G HSPAPlus", "4G LTE", "4G TDD", "4G FDD"}
    cat::HCatBuffer code;           ///< 错误码(保留)

    PppoeInfo()
        : vaild(false)
        , enable(false)
        , signal(0)
        , dbm(0)
        , insert(false)
    {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetPppoeInfo"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetApn"; }
    static cat::HCatBuffer GetMethod() { return "GetPppoeInfo"; }
    static cat::HCatBuffer SetMethod() { return "SetApn"; }
};
void from_xml(const HXml &xml, PppoeInfo &node);
void to_xml(HXml &xml, const PppoeInfo &node);



}


#endif // __HETHERNETINFO_H__
