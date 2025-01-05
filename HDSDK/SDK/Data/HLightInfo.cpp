

#include <HCatBuffer.h>
#include <Data/HLightInfo.h>
#include <string>


namespace sdk
{


void from_xml(const HXml &xml, LightInfo &node)
{
    
    cat::HCatBuffer mode = xml.at("mode").GetAttribute("value");
    if (mode == "ploys") {
        node.mode = LightInfo::kPloys;
    } else if (mode == "sensor") {
        node.mode = LightInfo::kSensor;
    } else {
        node.mode = LightInfo::kDefault;
    }

    node.defaultValue = xml.at("default").GetAttribute("value");
    node.sensor.min = xml.at("sensor").GetAttribute("min").ToInt();
    node.sensor.max = xml.at("sensor").GetAttribute("max").ToInt();
    node.sensor.time = xml.at("sensor").GetAttribute("time").ToInt();

    node.ployList.clear();
    if (xml.ContainsNodes("ploy") && xml.at("ploy").ContainsNodes("item")) {
        for (const auto &i : xml.at("ploy").at("item")) {
            LightInfo::Ploy item;
            item.enable = i.GetAttribute("enable") == "true";
            item.start = i.GetAttribute("start");
            item.percent = i.GetAttribute("percent").ToInt();
            node.ployList.emplace_back(std::move(item));
        }
    }
}



void to_xml(HXml &xml, const LightInfo &node)
{
    
    switch (node.mode) {
    case LightInfo::kDefault: xml["mode"] = {"value", "default"}; break;
    case LightInfo::kPloys: xml["mode"] = {"value", "ploys"}; break;
    case LightInfo::kSensor: xml["mode"] = {"value", "sensor"}; break;
    default:
        xml["mode"] = {"value", "default"};
        break;
    }

    xml["default"] = {"value", node.defaultValue};
    xml["sensor"] = {{"min", node.sensor.min},
                     {"max", node.sensor.max},
                     {"time", node.sensor.time}};
    for (const auto &i : node.ployList) {
        xml["ploy"].emplace_back("item", {{"enable", i.enable},
                                          {"start", i.start},
                                          {"percent", i.percent}});
    }
}


}
