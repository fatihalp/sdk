

#include <HCatBuffer.h>
#include <Data/HGeneralInfo.h>


namespace sdk
{


void to_xml(HXml &xml, const SystemVolumeInfo &data)
{
    xml["mode"] = HXml{"value", data.mode != SystemVolumeInfo::kDefault ? "ploys" : "default"};
    xml["volume"] = HXml{"percent", data.volume};
    for (const auto &i : data.ploys) {
        xml["ploy"].emplace_back("item", {{"percent", i.volume},
                                          {"enable", i.enable},
                                          {"start", i.time}});
    }
}


void from_xml(const HXml &xml, SystemVolumeInfo &data)
{
    data.mode = xml.at("mode").GetAttribute("value") == "ploys" ? SystemVolumeInfo::kPloys : SystemVolumeInfo::kDefault;
    data.volume = xml.at("volume").GetAttribute("percent");
    data.ploys.clear();
    if (xml.ContainsNodes("ploy", "item")) {
        for (const auto &i : xml.at("ploy").at("item")) {
            SystemVolumeInfo::Ploy item;
            item.enable = DataToType<bool>(i.GetAttribute("enable"));
            item.volume = i.GetAttribute("percent").ToInt();
            item.time = i.GetAttribute("start");
            data.ploys.emplace_back(std::move(item));
        }
    }
}


void to_xml(HXml &xml, const DeviceNameInfo &data)
{
    xml["name"] = {"value", data.name};
}


void from_xml(const HXml &xml, DeviceNameInfo &data)
{
    data.name = xml.at("name").GetAttribute("value");
}


void to_xml(HXml &xml, const SDKTcpServerInfo &data)
{   
    xml["server"] = {{"port", data.port},
                     {"host", data.host}};
}


void from_xml(const HXml &xml, SDKTcpServerInfo &data)
{   
    data.port = xml.at("server").GetAttribute("port").ToInt();
    data.host = xml.at("server").GetAttribute("host");
}

void from_xml(const HXml &xml, DeviceInfo &data)
{
    data.device.cpu = xml.at("device").GetAttribute("cpu");
    data.device.id = xml.at("device").GetAttribute("id");
    data.device.model = xml.at("device").GetAttribute("model");
    data.device.name = xml.at("device").GetAttribute("name");

    data.version.kernel = xml.at("version").GetAttribute("kernel");
    data.version.hardware = xml.at("version").GetAttribute("hardware");
    data.version.app = xml.at("version").GetAttribute("app");
    data.version.fpga = xml.at("version").GetAttribute("fpga");

    data.screen.rotation = xml.at("screen").GetAttribute("rotation").ToInt();
    data.screen.width = xml.at("screen").GetAttribute("width").ToInt();
    data.screen.height = xml.at("screen").GetAttribute("height").ToInt();
}

void to_xml(HXml &xml, const ScreenShot2 &data)
{
    xml["image"] = {{"width", data.width},
                    {"height", data.height}};
}

void from_xml(const HXml &xml, ScreenShot2 &data)
{
    data.rawData = cat::HCatBuffer::FromBase64(xml.at("image").GetAttribute("data"));
}

void to_xml(HXml &xml, const SwitchProgramInfo &data)
{
    if (data.guid.Empty() == false) {
        xml["program"] = {"guid", data.guid};
    } else {
        xml["program"] = {"index", data.index};
    }
}

void from_xml(const HXml &xml, SwitchProgramInfo &data)
{
    data.guid = xml.at("program").GetAttribute("guid");
}


}
