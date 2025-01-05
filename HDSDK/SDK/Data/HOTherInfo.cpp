

#include <Data/HOTherInfo.h>


namespace sdk
{


void from_xml(const HXml &xml, MulScreenSyncInfo &node)
{
    node.enable = xml.at("enable").GetAttribute("value") == "true";
}


void to_xml(HXml &xml, const MulScreenSyncInfo &node)
{
    xml["enable"] = {"value", node.enable ? "true" : "false"};
}


void from_xml(const HXml &xml, GpsRespondInfo &node)
{
    node.enable = xml.at("gps").GetAttribute("enable") == "true";
    node.delay = xml.at("gps").GetAttribute("delay").ToInt();
}


void to_xml(HXml &xml, const GpsRespondInfo &node)
{
    xml["gps"] = {{"enable", node.enable ? "true" : "false"},
                  {"delay", node.delay}};
}


}
