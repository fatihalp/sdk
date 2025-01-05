

#include <HCatBuffer.h>
#include <Data/HTimeInfo.h>


namespace sdk
{


void from_xml(const HXml &xml, TimeInfo &node)
{
    node.timeZone = xml.at("timezone").GetAttribute("value");
    node.summer = xml.at("summer").GetAttribute("enable") == "true";
    node.sync = xml.at("sync").GetAttribute("value");
    node.currTime = xml.at("time").GetAttribute("value");
    node.serverList.Clear();
    if (xml.ContainsNodes("server")) {
        node.serverList = xml.at("server").GetAttribute("list");
    }
}


void to_xml(HXml &xml, const TimeInfo &node)
{
    xml["timezone"] = {"value", node.timeZone};
    xml["summer"] = {{"enable", node.summer}};
    xml["sync"] = {"value", node.sync.Empty() ? "none" : node.sync};
    xml["time"] = {"value", node.currTime};
    xml["server"] = {"list", node.serverList};
    xml["rf"]["enable"] = {"value", node.rf.enable};
    xml["rf"]["master"] = {"value", node.rf.master};
    xml["rf"]["channel"] = {"value", node.rf.channel};
}


}
