

#include "HDSDK.h"
#include "HScreenFunctionInfo.h"
#include "HSensorInfo.h"
#include <SDKInfo.h>
#include <string>
#include <HCatBuffer.h>
#include <tinyxml2.h>
#include <HXml.h>
#include <Data/HSDKInfo.h>


namespace detail
{


template <typename _T>
cat::HCatBuffer GetNode(const _T &) { return _T::GetMethod(); }
template <typename _T>
cat::HCatBuffer SetNode(const _T &) { return _T::SetMethod(); }


}


ISDKInfo DLL_CALL CreateSDKInfo()
{
    return new sdk::HSDKInfo();
}

void DLL_CALL FreeSDKInfo(sdk::ISDKInfo info)
{
    delete info;
}


HBool DLL_CALL ParseXml(ISDKInfo info, const char *xml, int len)
{
    if (xml == nullptr || info == nullptr || len <= 0) {
        return HFalse;
    }

    tinyxml2::XMLDocument doc;
    doc.Parse(xml, len);
    if (doc.Error()) {
        return HFalse;
    }

    tinyxml2::XMLElement *sdk = doc.FirstChildElement("sdk");
    if (!sdk) {
        printf("read xml error, sdk tag not found\n");
        return HFalse;
    }

    tinyxml2::XMLElement *node = sdk->FirstChildElement("out");
    if (!node) {
        printf("read xml error, out tag not found\n");
        return HFalse;
    }

    while (node) {
        cat::HCatBuffer method(node->Attribute("method"));
        if (info->ParseInfo(node, method) == 0) {
            return HFalse;
        }

        node = node->NextSiblingElement("out");
    }

    return HTrue;
}

HBool UpdateItem(IHDProtocol protocol, ISDKInfo info, int updateItem)
{
    if (protocol == nullptr || info == nullptr) {
        return HFalse;
    }

    HXml xml;
    xml["sdk"] = {"guid", "##GUID"};

// 存在配置信息的Get项
#define GET_DATA_ITEM(item, obj) \
    case item: { \
        xml["sdk"]["in"] = {"method", detail::GetNode(obj)}; \
        xml["sdk"]["in"].ToXml(obj); \
    } break;

// 常规Get项
#define GET_ITEM(item, obj) \
    case item: { \
        xml["sdk"]["in"] = {"method", detail::GetNode(obj)}; \
    } break;

// 常规Set项
#define SET_ITEM(item, obj) \
    case item: { \
        xml["sdk"]["in"] = {"method", detail::SetNode(obj)}; \
        xml["sdk"]["in"].ToXml(obj); \
    } break;

// 多处理常规Set和Get系列
#define GET_SET_ITEM(getItem, setItem, obj) \
    GET_ITEM(getItem, obj); \
    SET_ITEM(setItem, obj);


    switch (updateItem) {
        GET_SET_ITEM(kGetLightInfo, kSetLightInfo, info->lightInfo);
        GET_SET_ITEM(kGetSystemVolumeInfo, kSetSystemVolumeInfo, info->systemVolumeInfo);
        GET_SET_ITEM(kGetTcpServerInfo, kSetTcpServerInfo, info->tcpSercerInfo);
        GET_SET_ITEM(kGetTimeInfo, kSetTimeInfo, info->timeInfo);
        GET_SET_ITEM(kGetEthInfo, kSetEthInfo, info->ethInfo);
        GET_SET_ITEM(kGetWifiInfo, kSetWifiInfo, info->wifiInfo);
        GET_SET_ITEM(kGetPppoeInfo, kSetPppoeInfo, info->pppoeInfo);
        GET_SET_ITEM(kGetDeviceNameInfo, kSetDeviceNameInfo, info->deviceNameInfo);
        GET_SET_ITEM(kGetSwitchTimeInfo, kSetSwitchTimeInfo, info->switchTimeInfo);
        GET_SET_ITEM(kGetRelayInfo, kSetRelayInfo, info->relayInfo);
        GET_SET_ITEM(kGetCurrProgramGuid, kSetCurrProgramGuid, info->switchProgramInfo);

        GET_ITEM(kGetDeviceInfo, info->deviceInfo);
        GET_DATA_ITEM(kGetScreenShot, info->screenShot2);
    default:
        return HFalse;
    }

    std::string buff = xml.Dump();
    return SendXml(protocol, buff.c_str(), buff.size()) ? HTrue : HFalse;

#undef GET_DATA_ITEM
#undef GET_ITEM
#undef SET_ITEM
#undef SET_ITEM_INFO
#undef GET_SET_ITEM
}

void SetLightInfo(ISDKInfo info, int mode, int defaultModeLight)
{
    info->lightInfo.mode = static_cast<sdk::LightInfo::eMode>(mode);
    info->lightInfo.defaultValue = cat::HCatBuffer::Number(defaultModeLight);
}

void SetLightInfoSensor(ISDKInfo info, int min, int max, int time)
{
    info->lightInfo.sensor.min = std::max(1, min);
    info->lightInfo.sensor.max = std::min(max, 100);
    info->lightInfo.sensor.time = std::max(5, std::min(15, time));
}

void AddLightInfoPloy(ISDKInfo info, HBool enable, const char *startTime, int percent)
{
    sdk::LightInfo::Ploy item;
    item.enable = enable;
    item.percent = percent;
    item.start = cat::HCatBuffer(startTime);

    info->lightInfo.ployList.emplace_back(std::move(item));
}

void SetLightInfoPloy(ISDKInfo info, int index, HBool enable, const char *startTime, int percent)
{
    if (index < 0 || index >= static_cast<int>(info->lightInfo.ployList.size())) {
        return ;
    }

    info->lightInfo.ployList[index].enable = enable != HFalse;
    info->lightInfo.ployList[index].start = cat::HCatBuffer(startTime);
    info->lightInfo.ployList[index].percent = percent;
}

void ClearLightInfoPloy(ISDKInfo info)
{
    info->lightInfo.ployList.clear();
}

int GetLightInfoMode(ISDKInfo info)
{
    return info->lightInfo.mode;
}

int GetLightInfoDefaultLight(ISDKInfo info)
{
    return info->lightInfo.defaultValue.ToInt();
}

int GetLightInfoPloySize(ISDKInfo info)
{
    return info->lightInfo.ployList.size();
}

int GetLightInfoPloyEnable(ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->lightInfo.ployList.size())) {
        return 0;
    }

    return info->lightInfo.ployList.at(index).enable;
}

int GetLightInfoPloyPercent(ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->lightInfo.ployList.size())) {
        return 0;
    }

    return info->lightInfo.ployList.at(index).percent;
}

const char *GetLightInfoPloyStart(ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->lightInfo.ployList.size())) {
        return nullptr;
    }

    return info->lightInfo.ployList.at(index).start.ConstData();
}

int GetLightInfoSensorMax(ISDKInfo info)
{
    return info->lightInfo.sensor.max;
}

int GetLightInfoSensorMin(ISDKInfo info)
{
    return info->lightInfo.sensor.min;
}

int GetLightInfoSensorTime(ISDKInfo info)
{
    return info->lightInfo.sensor.time;
}

void SetSystemVolumeInfo(ISDKInfo info, int mode, int volume)
{
    info->systemVolumeInfo.mode = mode;
    info->systemVolumeInfo.volume = cat::HCatBuffer::Number(std::min(100, std::max(volume, 0)));
}

void AddSystemVolumeInfoPloy(ISDKInfo info, int enable, const char *time, int volume)
{
    sdk::SystemVolumeInfo::Ploy item;
    item.enable = enable;
    item.volume = std::min(100, std::max(volume, 0));
    item.time = cat::HCatBuffer(time);
    info->systemVolumeInfo.ploys.emplace_back(std::move(item));
}

void ClearSystemVolumeInfoPloy(ISDKInfo info)
{
    info->systemVolumeInfo.ploys.clear();
}

int GetSystemVolumeInfoMode(ISDKInfo info)
{
    return info->systemVolumeInfo.mode;
}

int GetSystemVolumeInfoVolume(ISDKInfo info)
{
    return info->systemVolumeInfo.volume.ToInt();
}

int GetSystemVolumeInfoPloySize(ISDKInfo info)
{
    return info->systemVolumeInfo.ploys.size();
}

int GetSystemVolumeInfoPloyEnable(ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->systemVolumeInfo.ploys.size())) {
        return HFalse;
    }

    return info->systemVolumeInfo.ploys.at(index).enable ? HTrue : HFalse;
}

const char *GetSystemVolumeInfoPloyTime(ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->systemVolumeInfo.ploys.size())) {
        return nullptr;
    }

    return info->systemVolumeInfo.ploys.at(index).time.ConstData();
}

int GetSystemVolumeInfoPloyVolume(ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->systemVolumeInfo.ploys.size())) {
        return 0;
    }

    return info->systemVolumeInfo.ploys.at(index).volume;
}

void SetTcpServerInfo(ISDKInfo info, const char *ip, huint16 port)
{
    info->tcpSercerInfo.host = cat::HCatBuffer(ip);
    info->tcpSercerInfo.port = port;
}

const char *GetTcpServerInfoIp(ISDKInfo info)
{
    return info->tcpSercerInfo.host.ConstData();
}

huint16 GetTcpServerInfoPort(ISDKInfo info)
{
    return info->tcpSercerInfo.port;
}

void SetTimeInfo(ISDKInfo info, const char *timeZone, int summer, const char *sync, const char *currTime, const char *serverList)
{
    info->timeInfo.timeZone = cat::HCatBuffer(timeZone);
    info->timeInfo.summer = summer == HTrue;
    info->timeInfo.sync = cat::HCatBuffer(sync);
    info->timeInfo.currTime = cat::HCatBuffer(currTime);
    info->timeInfo.serverList = cat::HCatBuffer(serverList);
}

const char *GetTimeInfoTimeZone(ISDKInfo info)
{
    return info->timeInfo.timeZone.ConstData();
}

int GetTimeInfoSummer(ISDKInfo info)
{
    return info->timeInfo.summer ? HTrue : HFalse;
}

const char *GetTimeInfoSync(ISDKInfo info)
{
    return info->timeInfo.sync.ConstData();
}

const char *GetTimeInfoCurrTime(ISDKInfo info)
{
    return info->timeInfo.currTime.ConstData();
}

const char *GetTimeInfoServerList(ISDKInfo info)
{
    return info->timeInfo.serverList.ConstData();
}

void SetEthInfo(sdk::ISDKInfo info, HBool dhcp, const char *ip, const char *netmask, const char *gateway, const char *dns)
{
    info->ethInfo.dhcp = dhcp != HFalse;
    info->ethInfo.ip = cat::HCatBuffer(ip);
    info->ethInfo.netmask = cat::HCatBuffer(netmask);
    info->ethInfo.gateway = cat::HCatBuffer(gateway);
    info->ethInfo.dns = cat::HCatBuffer(dns);
}

HBool GetEhtInfoDhcp(sdk::ISDKInfo info)
{
    return info->ethInfo.dhcp ? HTrue : HFalse;
}

const char *GetEhtInfoIp(sdk::ISDKInfo info)
{
    return info->ethInfo.ip.ConstData();
}

const char *GetEhtInfoNetmask(sdk::ISDKInfo info)
{
    return info->ethInfo.netmask.ConstData();
}

const char *GetEhtInfoGateway(sdk::ISDKInfo info)
{
    return info->ethInfo.gateway.ConstData();
}

const char *GetEhtInfoDns(sdk::ISDKInfo info)
{
    return info->ethInfo.dns.ConstData();
}

void SetWifiInfo(sdk::ISDKInfo info, int mode)
{
    if (mode == 1) {
        info->wifiInfo.mode = "station";
    } else {
        info->wifiInfo.mode = "ap";
    }
}

void SetWifiInfoAp(sdk::ISDKInfo info, const char *ssid, const char *password, const char *ip)
{
    info->wifiInfo.ap.ssid = cat::HCatBuffer(ssid);
    info->wifiInfo.ap.password = cat::HCatBuffer(password);
    info->wifiInfo.ap.ipAddress = cat::HCatBuffer(ip);
    info->wifiInfo.ap.channel = 0;
}

void SetWifiInfoStation(sdk::ISDKInfo info, const char *ssid, const char *password, int dhcp)
{
    info->wifiInfo.stationIndex = 0;
    if (info->wifiInfo.station.size() > 1) {
        info->wifiInfo.station.clear();
    }
    if (info->wifiInfo.station.empty()) {
        info->wifiInfo.station.emplace_back(sdk::WifiInfo::StationConfig());
    }

    info->wifiInfo.station.front().dhcp = dhcp != HFalse;
    info->wifiInfo.station.front().ssid = cat::HCatBuffer(ssid);
    info->wifiInfo.station.front().password = cat::HCatBuffer(password);
}

void SetWifiInfoStationNet(sdk::ISDKInfo info, const char *ip, const char *mask, const char *gateway, const char *dns)
{
    info->wifiInfo.stationIndex = 0;
    if (info->wifiInfo.station.size() > 1) {
        info->wifiInfo.station.clear();
    }
    if (info->wifiInfo.station.empty()) {
        info->wifiInfo.station.emplace_back(sdk::WifiInfo::StationConfig());
    }

    info->wifiInfo.station.front().ip = cat::HCatBuffer(ip);
    info->wifiInfo.station.front().mask = cat::HCatBuffer(mask);
    info->wifiInfo.station.front().gateway = cat::HCatBuffer(gateway);
    info->wifiInfo.station.front().dns = cat::HCatBuffer(dns);
}

int GetWifiInfoMode(sdk::ISDKInfo info)
{
    if (info->wifiInfo.mode == "station") {
        return 1;
    }

    return 0;
}

const char *GetWifiInfoApSsid(sdk::ISDKInfo info)
{
    return info->wifiInfo.ap.ssid.ConstData();
}

const char *GetWifiInfoApPassword(sdk::ISDKInfo info)
{
    return info->wifiInfo.ap.password.ConstData();
}

const char *GetWifiInfoApIp(sdk::ISDKInfo info)
{
    return info->wifiInfo.ap.ipAddress.ConstData();
}

const char *GetWifiInfoStationSsid(sdk::ISDKInfo info)
{
    return info->wifiInfo.GetCurrentStation().ssid.ConstData();
}

HBool GetWifiInfoStationDhcp(sdk::ISDKInfo info)
{
    return info->wifiInfo.GetCurrentStation().dhcp ? HTrue : HFalse;
}

const char *GetWifiInfoStationIp(sdk::ISDKInfo info)
{
    return info->wifiInfo.GetCurrentStation().ip.ConstData();
}

const char *GetWifiInfoStationMask(sdk::ISDKInfo info)
{
    return info->wifiInfo.GetCurrentStation().mask.ConstData();
}

const char *GetWifiInfoStationGateway(sdk::ISDKInfo info)
{
    return info->wifiInfo.GetCurrentStation().gateway.ConstData();
}

const char *GetWifiInfoStationDns(sdk::ISDKInfo info)
{
    return info->wifiInfo.GetCurrentStation().dns.ConstData();
}

void SetPppoeInfoApn(sdk::ISDKInfo info, const char *apn)
{
    info->pppoeInfo.apn = cat::HCatBuffer(apn);
}

HBool GetPppoeInfoVaild(sdk::ISDKInfo info)
{
    return info->pppoeInfo.vaild ? HTrue : HFalse;
}

const char *GetPppoeInfoApn(sdk::ISDKInfo info)
{
    return info->pppoeInfo.apn.ConstData();
}

void SetDeviceNameInfo(sdk::ISDKInfo info, const char *name)
{
    info->deviceNameInfo.name = cat::HCatBuffer(name);
}

const char *GetDeviceNameInfo(sdk::ISDKInfo info)
{
    return info->deviceNameInfo.name.ConstData();
}

void SetSwitchTimeInfo(sdk::ISDKInfo info, int mode, int enable)
{
    info->switchTimeInfo.mode = mode;
    info->switchTimeInfo.ployEnable = enable != HFalse;
}

void AddSwitchTimeInfoItem(sdk::ISDKInfo info, HBool enable, const char *start, const char *end)
{
    sdk::SwitchTimeInfo::ployItem item;
    item.enable = enable != HFalse;
    item.start = cat::HCatBuffer(start);
    item.end = cat::HCatBuffer(end);
    info->switchTimeInfo.ploys.emplace_back(std::move(item));
}

void ClearSwitchTimeInfoItem(sdk::ISDKInfo info)
{
    info->switchTimeInfo.ploys.clear();
}

HBool SetSwitchTimeInfoItem(sdk::ISDKInfo info, int index, HBool enable, const char *start, const char *end)
{
    if (index < 0 || index >= static_cast<int>(info->switchTimeInfo.ploys.size())) {
        return HFalse;
    }

    info->switchTimeInfo.ploys[index].enable = enable != HFalse;
    info->switchTimeInfo.ploys[index].start = cat::HCatBuffer(start);
    info->switchTimeInfo.ploys[index].end = cat::HCatBuffer(end);
    return HTrue;
}

void AddSwitchTimeInfoWeekItem(sdk::ISDKInfo info, int week, HBool openAllDay, const char *start, const char *end)
{
    if (week < 0 || week >= static_cast<int>(sdk::SwitchTimeInfo::GetWeekMax())) {
        return ;
    }

    sdk::SwitchTimeInfo::weekItem item;
    item.start = cat::HCatBuffer(start);
    item.end = cat::HCatBuffer(end);
    info->switchTimeInfo.GetWeekItem(week).openAllDay = openAllDay != HFalse;
    info->switchTimeInfo.GetWeekItem(week).ploys.emplace_back(std::move(item));
}

void ClearSwitchTimeInfoWeekItem(sdk::ISDKInfo info, int week)
{
    if (week < 0 || week >= static_cast<int>(sdk::SwitchTimeInfo::GetWeekMax())) {
        return ;
    }

    info->switchTimeInfo.GetWeekItem(week).ploys.clear();
}

void SetSwitchTimeInfoWeekItem(sdk::ISDKInfo info, int week, int index, HBool openAllDay, const char *start, const char *end)
{
    if (week < 0 || week >= static_cast<int>(sdk::SwitchTimeInfo::GetWeekMax())) {
        return ;
    }

    if (index < 0 || index >= static_cast<int>(info->switchTimeInfo.weekPloys.size())) {
        return ;
    }

    info->switchTimeInfo.GetWeekItem(week).openAllDay = openAllDay != HFalse;
    info->switchTimeInfo.GetWeekItem(week).ploys[index].start = cat::HCatBuffer(start);
    info->switchTimeInfo.GetWeekItem(week).ploys[index].end = cat::HCatBuffer(end);
}

int GetSwitchTimeInfoItemSize(sdk::ISDKInfo info)
{
    return info->switchTimeInfo.ploys.size();
}

int GetSwitchTimeInfoWeekItemSize(sdk::ISDKInfo info, int week)
{
    if (week < 0 || week >= static_cast<int>(sdk::SwitchTimeInfo::GetWeekMax())) {
        return 0;
    }

    return info->switchTimeInfo.GetWeekItem(week).ploys.size();
}

HBool GetSwitchTimeInfoEnable(sdk::ISDKInfo info)
{
    return info->switchTimeInfo.ployEnable ? HTrue : HFalse;
}

HBool GetSwitchTimeInfoItemEnable(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->switchTimeInfo.ploys.size())) {
        return HFalse;
    }

    return info->switchTimeInfo.ploys.at(index).enable ? HTrue : HFalse;
}

const char *GetSwitchTimeInfoItemStart(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->switchTimeInfo.ploys.size())) {
        return nullptr;
    }

    return info->switchTimeInfo.ploys.at(index).start.ConstData();
}

const char *GetSwitchTimeInfoItemEnd(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->switchTimeInfo.ploys.size())) {
        return nullptr;
    }

    return info->switchTimeInfo.ploys.at(index).end.ConstData();
}

void SetRelayInfoItem(sdk::ISDKInfo info, int index, const char *name, HBool useSwitch)
{
    info->relayInfo.InitRelayList();
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return ;
    }

    info->relayInfo.relayList[index].name = cat::HCatBuffer(name);
    info->relayInfo.relayList[index].useSwitch = useSwitch != HFalse;
}

void AddRelayInfoItemPloy(sdk::ISDKInfo info, int index, const char *start, const char *end)
{
    info->relayInfo.InitRelayList();
    if (index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return ;
    }

    sdk::RelayInfo::ployItem item;
    item.start = cat::HCatBuffer(start);
    item.end = cat::HCatBuffer(end);
    info->relayInfo.relayList[index].ploys.emplace_back(std::move(item));
}

void ClearRelayInfoItemPloy(sdk::ISDKInfo info, int index)
{
    info->relayInfo.InitRelayList();
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return ;
    }

    info->relayInfo.relayList[index].ploys.clear();
}

void SetRelayInfoItemPloyItem(sdk::ISDKInfo info, int index, int itemIndex, const char *start, const char *end)
{
    info->relayInfo.InitRelayList();
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return ;
    }

    if (itemIndex < 0 || itemIndex >= static_cast<int>(info->relayInfo.relayList.at(index).ploys.size())) {
        return ;
    }

    info->relayInfo.relayList[index].ploys[itemIndex].start = cat::HCatBuffer(start);
    info->relayInfo.relayList[index].ploys[itemIndex].end = cat::HCatBuffer(end);
}

void SetRelayInfoInternal(sdk::ISDKInfo info, const char *name, HBool useSwitch)
{
    info->relayInfo.internal.name = cat::HCatBuffer(name);
    info->relayInfo.internal.useSwitch = useSwitch != HFalse;
}

void AddRelayInfoInternalPloy(sdk::ISDKInfo info, const char *start, const char *end)
{
    sdk::RelayInfo::ployItem item;
    item.start = cat::HCatBuffer(start);
    item.end = cat::HCatBuffer(end);
    info->relayInfo.internal.ploys.emplace_back(std::move(item));
}

void ClearRelayInfoInternalPloy(sdk::ISDKInfo info)
{
    info->relayInfo.internal.ploys.clear();
}

void SetRelayInfoInternalPloyItem(sdk::ISDKInfo info, int index, const char *start, const char *end)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.internal.ploys.size())) {
        return ;
    }

    info->relayInfo.internal.ploys[index].start = cat::HCatBuffer(start);
    info->relayInfo.internal.ploys[index].end = cat::HCatBuffer(end);
}

int GetRelayInfoStatus(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return 0;
    }

    return info->relayInfo.relayList.at(index).relayStatus;
}

const char *GetRelayInfoName(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return nullptr;
    }

    return info->relayInfo.relayList.at(index).name.ConstData();
}

HBool GetRelayInfoUseSwitch(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return HFalse;
    }

    return info->relayInfo.relayList.at(index).useSwitch ? HTrue : HFalse;
}

int GetRelayInfoItemPloySize(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return 0;
    }

    return info->relayInfo.relayList.at(index).ploys.size();
}

const char *GetRelayInfoItemPloyStart(sdk::ISDKInfo info, int index, int itemIndex)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return nullptr;
    }

    if (itemIndex < 0 || itemIndex >= static_cast<int>(info->relayInfo.relayList.at(index).ploys.size())) {
        return nullptr;
    }

    return info->relayInfo.relayList.at(index).ploys.at(itemIndex).start.ConstData();
}

const char *GetRelayInfoItemPloyEnd(sdk::ISDKInfo info, int index, int itemIndex)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.relayList.size())) {
        return nullptr;
    }

    if (itemIndex < 0 || itemIndex >= static_cast<int>(info->relayInfo.relayList.at(index).ploys.size())) {
        return nullptr;
    }

    return info->relayInfo.relayList.at(index).ploys.at(itemIndex).end.ConstData();
}

int GetRelayInfoInternalPloySize(sdk::ISDKInfo info)
{
    return info->relayInfo.internal.ploys.size();
}

const char *GetRelayInfoInternalPloyStart(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.internal.ploys.size())) {
        return nullptr;
    }

    return info->relayInfo.internal.ploys.at(index).start.ConstData();
}

const char *GetRelayInfoInternalPloyEnd(sdk::ISDKInfo info, int index)
{
    if (index < 0 || index >= static_cast<int>(info->relayInfo.internal.ploys.size())) {
        return nullptr;
    }

    return info->relayInfo.internal.ploys.at(index).end.ConstData();
}

void SetCurrProgramGuid(sdk::ISDKInfo info, const char *guid, int index)
{
    info->switchProgramInfo.guid = cat::HCatBuffer(guid);
    info->switchProgramInfo.index = index;
}

const char *GetCurrProgramGuid(sdk::ISDKInfo info)
{
    return info->switchProgramInfo.guid.ConstData();
}

const char *GetDevceInfoId(ISDKInfo info)
{
    return info->deviceInfo.device.id.ConstData();
}

const char *GetDevceInfoName(ISDKInfo info)
{
    return info->deviceInfo.device.name.ConstData();
}

const char *GetDevceInfoAppVersion(ISDKInfo info)
{
    return info->deviceInfo.version.app.ConstData();
}

const char *GetDevceInfoFpgaVersion(ISDKInfo info)
{
    return info->deviceInfo.version.fpga.ConstData();
}

int GetDevceInfoScreenRotation(ISDKInfo info)
{
    return info->deviceInfo.screen.rotation;
}

int GetDevceInfoScreenWidth(ISDKInfo info)
{
    return info->deviceInfo.screen.width;
}

int GetDevceInfoScreenHeight(ISDKInfo info)
{
    return info->deviceInfo.screen.height;
}

void SetScreenShot(ISDKInfo info, int width, int height)
{
    info->screenShot2.width = width;
    info->screenShot2.height = height;
}

const char *GetScreenShot(ISDKInfo info)
{
    return info->screenShot2.rawData.ConstData();
}

int GetScreenShotSize(sdk::ISDKInfo info)
{
    return info->screenShot2.rawData.Size();
}
