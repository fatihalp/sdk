

#include <Data/HEthernetInfo.h>


namespace sdk
{


void from_xml(const HXml &xml, Eth0Info &node)
{
    node.dhcp = xml.at("eth").at("dhcp").GetAttribute("auto") == "true";
    node.ip = xml.at("eth").at("address").GetAttribute("ip");
    node.netmask = xml.at("eth").at("address").GetAttribute("netmask");
    node.gateway = xml.at("eth").at("address").GetAttribute("gateway");
    node.dns = xml.at("eth").at("address").GetAttribute("dns");
}


void to_xml(HXml &xml, const Eth0Info &node)
{
    xml["eth"] = {"valid", "true"};
    xml["eth"]["enable"] = {"value", "true"};
    xml["eth"]["dhcp"] = {"auto", node.dhcp};
    xml["eth"]["address"] = {{"ip", node.ip},
                             {"netmask", node.netmask},
                             {"gateway", node.gateway},
                             {"dns", node.dns}};
}


void from_xml(const HXml &xml, WifiInfo &node)
{
    node.mode = xml.at("mode").GetAttribute("value");
    node.ap.ssid = xml.at("ap").at("ssid").GetAttribute("value");
    node.ap.password = xml.at("ap").at("passwd").GetAttribute("value");
    node.ap.ipAddress = xml.at("ap").at("address").GetAttribute("ip");
    if (xml.at("ap").ContainsNodes("channel")) {
        node.ap.channel = xml.at("ap").at("channel").GetAttribute("value").ToInt();
    }

    node.stationIndex = xml.at("station").at("current").GetAttribute("index").ToInt();

    node.station.clear();
    for (const auto &i : xml.at("station").at("list").at("item")) {
        WifiInfo::StationConfig info;
        info.ssid = i.at("ssid").GetAttribute("value");
        info.password = i.at("passwd").GetAttribute("value");
        info.dhcp = i.at("dhcp").GetAttribute("auto") != "false";
        info.ip = i.at("address").GetAttribute("ip");
        info.mask = i.at("address").GetAttribute("netmask");
        info.gateway = i.at("address").GetAttribute("gateway");
        info.dns = i.at("address").GetAttribute("dns");
        node.station.emplace_back(std::move(info));
    }
}


void to_xml(HXml &xml, const WifiInfo &node)
{
    xml["mode"] = {"value", node.mode.GetConstString()};
    xml["ap"]["ssid"] = {"value", node.ap.ssid.GetConstString()};
    xml["ap"]["passwd"] = {"value", node.ap.password.GetConstString()};
    xml["ap"]["channel"] = {"value", node.ap.channel};
    xml["ap"]["encryption"] = {"value", "WPA-PSK"};
    xml["ap"]["dhcp"] = {"auto", "true"};
    xml["ap"]["address"] = {{"ip", node.ap.ipAddress.GetConstString()},
                            {"netmask", ""},
                            {"gateway", ""},
                            {"dns", ""}};

    xml["station"]["current"] = {"index", node.stationIndex};
    for (const auto &i : node.station) {
        HXml &item = xml["station"]["list"].NewChild("item");
        item["ssid"] = {"value", i.ssid};
        item["passwd"] = {"value", i.password};
        item["signal"] = {"value", 0};
        item["apmac"] = {"value", i.mac};
        item["dhcp"] = {"auto", i.dhcp ? "true" : "false"};
        item["address"] = {{"ip", i.ip},
                           {"netmask", i.mask},
                           {"gateway", i.gateway},
                           {"dns", i.dns}};
    }
}


void from_xml(const HXml &xml, PppoeInfo &node)
{
    node.vaild = xml.at("pppoe").GetAttribute("valid") == "true";
    node.enable = xml.at("pppoe").at("enable").GetAttribute("value") == "true";
    node.apn = xml.at("pppoe").at("apn").GetAttribute("value");
    node.manufacturer = xml.at("pppoe").at("manufacturer").GetAttribute("value");
    node.version = xml.at("pppoe").at("version").GetAttribute("value");
    node.model = xml.at("pppoe").at("model").GetAttribute("value");
    node.imei = xml.at("pppoe").at("imei").GetAttribute("value");
    node.number = xml.at("pppoe").at("number").GetAttribute("value");
    node.operators = xml.at("pppoe").at("operators").GetAttribute("value");
    node.signal = xml.at("pppoe").at("signal").GetAttribute("value").ToInt();
    node.dbm = xml.at("pppoe").at("dbm").GetAttribute("value").ToInt();
    node.insert = xml.at("pppoe").at("insert").GetAttribute("value") == "true";
    node.status = xml.at("pppoe").at("status").GetAttribute("value");
    node.network = xml.at("pppoe").at("network").GetAttribute("value");
    node.code = xml.at("pppoe").at("code").GetAttribute("value");
}


void to_xml(HXml &xml, const PppoeInfo &node)
{
    xml["apn"] = {"value", node.apn};
}

const WifiInfo::StationConfig &WifiInfo::GetCurrentStation()
{
    if (stationIndex < 0 || stationIndex >= static_cast<int>(station.size())) {
        stationIndex = 0;
        station.clear();
        station.emplace_back(StationConfig());
    }
    return station.front();
}


}
