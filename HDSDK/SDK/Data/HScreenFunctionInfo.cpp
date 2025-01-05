

#include <Data/HScreenFunctionInfo.h>


namespace sdk
{


void from_xml(const HXml &xml, SwitchTimeInfo &node)
{
    node.ploys.clear();
    node.weekPloys.clear();
    node.open = xml.at("open").GetAttribute("enable") == "true";
    node.ployEnable = xml.at("ploy").GetAttribute("enable") == "true";
    node.mode = xml.at("ploy").GetAttribute("mode").ToInt();
    do {
        if (xml.at("ploy").ContainsNodes("item") == false) {
            break;
        }

        for (const auto &i : xml.at("ploy").at("item")) {
            SwitchTimeInfo::ployItem item;
            item.enable = i.GetAttribute("enable") == "true";
            item.start = i.GetAttribute("start");
            item.end = i.GetAttribute("end");
            node.ploys.emplace_back(std::move(item));
        }
    } while (false);

    do {
        if (xml.at("ploy").ContainsNodes("weekItem") == false) {
            break;
        }

        for (const auto &i : xml.at("ploy").at("weekItem")) {
            SwitchTimeInfo::weekPloyItem item;
            item.openAllDay = i.GetAttribute("openAllDay").ToInt();
            item.week = i.GetAttribute("week").ToInt();

            if (i.ContainsNodes("item")) {
                for (const auto &j : i.at("item")) {
                    SwitchTimeInfo::weekItem itemTime;
                    itemTime.start = j.GetAttribute("start");
                    itemTime.end = j.GetAttribute("end");
                    item.ploys.emplace_back(std::move(itemTime));
                }
            }

            node.weekPloys.emplace_back(std::move(item));
        }
    } while (false);
}



void to_xml(HXml &xml, const SwitchTimeInfo &node)
{    
    xml["open"] = {"enable", node.ployEnable ? "true" : "false"};
    xml["ploy"] = {{"mode", node.mode},
                   {"enbale", node.ployEnable}};

    for (const auto &i : node.ploys) {
        xml["ploy"].emplace_back("item", {{"enable", i.enable},
                                          {"start", i.start},
                                          {"end", i.end}});
    }

    for (const auto &i : node.weekPloys) {
        HXml &item = xml["ploy"].NewChild("weekItem");
        item = {{"openAllDay", i.openAllDay ? 1 : 0},
                {"week", i.week}};


        for (const auto &j : i.ploys) {
            item.emplace_back("item", {{"start", j.start},
                                       {"end", j.end}});
        }
    }
}


void SwitchTimeInfo::InitWeekPloys()
{
    std::size_t week = 0;
    const std::size_t weekNum = GetWeekMax();
    for (auto i = weekPloys.begin(); i != weekPloys.end();) {
        if (week & (1 << i->week)) {
            i = weekPloys.erase(i);
            continue;
        }
        week |= 1 << i->week;
        ++i;
    }

    while (weekPloys.size() < weekNum) {
        weekPloyItem item;
        for (std::size_t i = 0; i < weekNum; ++i) {
            if (week & (1 << i)) {
                continue;
            }

            week |= 1 << i;
            item.week = i;
        }
        weekPloys.emplace_back(std::move(item));
    }
}

SwitchTimeInfo::weekPloyItem &SwitchTimeInfo::GetWeekItem(int week)
{
    if (weekPloys.size() < GetWeekMax()) {
        InitWeekPloys();
    }

    for (auto &i : weekPloys) {
        if (i.week == week) {
            return i;
        }
    }

    return weekPloys.front();
}


}

