

#include <Data/HSensorInfo.h>


// 外置继电器最大是6个, 常规系列那边第七个是内置继电器
#define MAX_RELAY_NUM   (6)


namespace sdk
{


void from_xml(const HXml &xml, SensorInfo &node)
{
    node.luminance = xml.at("luminance").GetAttribute("connect") == "true";
    node.temp1 = xml.at("temp1").GetAttribute("connect") == "true";
    node.humidity = xml.at("humidity").GetAttribute("connect") == "true";
    node.temp2 = xml.at("temp2").GetAttribute("connect") == "true";
    node.telecontroller = xml.at("telecontroller").GetAttribute("connect") == "true";
    node.gps = xml.at("gps").GetAttribute("connect") == "true";
    node.windSpeed = xml.at("windSpeed").GetAttribute("connect") == "true";
    node.windDirection = xml.at("windDirection").GetAttribute("connect") == "true";
    node.noise = xml.at("noise").GetAttribute("connect") == "true";
    node.pressure = xml.at("pressure").GetAttribute("connect") == "true";
    node.lightIntensity = xml.at("lightIntensity").GetAttribute("connect") == "true";
    node.rainfall = xml.at("rainfall").GetAttribute("connect") == "true";
    node.co2 = xml.at("co2").GetAttribute("connect") == "true";
    node.pm2d5 = xml.at("pm2d5").GetAttribute("connect") == "true";
    node.pm10 = xml.at("pm10").GetAttribute("connect") == "true";
}


void from_xml(const HXml &xml, GpsInfo &node)
{
    node.east = xml.at("gps").GetAttribute("east") == "true";
    node.north = xml.at("gps").GetAttribute("north") == "true";
    node.longitude = xml.at("gps").GetAttribute("longitude").ToFloat();
    node.latitude  = xml.at("gps").GetAttribute("latitude ").ToFloat();
    node.counts = xml.at("gps").GetAttribute("counts").ToInt();
    node.speed  = xml.at("gps").GetAttribute("speed ").ToFloat();
    node.direction  = xml.at("gps").GetAttribute("direction").ToFloat();
}


void from_xml(const HXml &xml, RelayInfo &node)
{
    node.relayList.clear();
    node.internal.ploys.clear();

    for (const auto &i : xml.at("item")) {
        RelayInfo::realyItem item;
        item.name = i.GetAttribute("name");
        item.relayStatus = i.GetAttribute("relayStatus").ToInt();
        item.useSwitch = DataToType<bool>(i.GetAttribute("useSwitch"));
        if (i.ContainsNodes("time") == false) {
            node.relayList.emplace_back(std::move(item));
            continue;
        }

        for (const auto &j : i.at("time")) {
            RelayInfo::ployItem timeItem;
            timeItem.start = j.GetAttribute("start");
            timeItem.end = j.GetAttribute("end");
            item.ploys.emplace_back(std::move(timeItem));
        }
        node.relayList.emplace_back(std::move(item));
    }

    do {
        if (xml.ContainsNodes("internal") == false) {
            break;
        }

        node.internal.name = xml.at("internal").GetAttribute("name");
        node.internal.relayStatus = xml.at("internal").GetAttribute("relayStatus").ToInt();
        node.internal.useSwitch = DataToType<bool>(xml.at("internal").GetAttribute("useSwitch"));
        if (xml.at("internal").ContainsNodes("time") == false) {
            break;
        }

        for (const auto &i : xml.at("internal").at("time")) {
            RelayInfo::ployItem timeItem;
            timeItem.start = i.GetAttribute("start");
            timeItem.end = i.GetAttribute("end");
            node.internal.ploys.emplace_back(std::move(timeItem));
        }
    } while (false);
}



void to_xml(HXml &xml, const RelayInfo &node)
{
    for (const auto &i : node.relayList) {
        HXml &item = xml.NewChild("item");
        item = {{"relayStatus", i.relayStatus},
                {"name", i.name},
                {"useSwitch", i.useSwitch ? 1 : 0}};

        for (const auto &j : i.ploys) {
            item.NewChild("time") = {{"start", j.start},
                                     {"end", j.end}};
        }
    }

    for (auto i = node.relayList.size(); i < MAX_RELAY_NUM; ++i) {
        xml.NewChild("item") = {{"relayStatus", "0"},
                                {"name", ""},
                                {"useSwitch", "0"}};
    }

    xml["internal"] = {{"relayStatus", node.internal.relayStatus},
                       {"name", node.internal.name},
                       {"useSwitch", node.internal.useSwitch ? 1 : 0}};

    for (const auto &i : node.internal.ploys) {
        xml["internal"].NewChild("time") = {{"start", i.start},
                                            {"end", i.end}};
    }
}

void RelayInfo::InitRelayList()
{
    while (relayList.size() < MAX_RELAY_NUM) {
        relayList.emplace_back(realyItem());
    }
}



}
