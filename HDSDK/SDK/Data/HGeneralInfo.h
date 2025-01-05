

#ifndef __HGENERALINFO_H__
#define __HGENERALINFO_H__



#include <HCatBuffer.h>
#include <HXml.h>
#include <vector>


namespace sdk
{


///< 系统音量
struct SystemVolumeInfo
{
    enum eMode {
        kDefault,
        kPloys,
    };
    struct Ploy {
        bool enable;            ///< 是否启用时间段
        cat::HCatBuffer time;   ///< 开始时间 "HH:mm:ss"
        int volume;             ///< 0-100
        Ploy() : enable(false), volume(0) {}
    };

    int mode;
    cat::HCatBuffer volume;     ///< 0-100
    std::vector<Ploy> ploys;    ///< 分时音量

    SystemVolumeInfo()
        : mode(kDefault)
    {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetSystemVolume"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetSystemVolume"; }
    static cat::HCatBuffer GetMethod() { return "GetSystemVolume"; }
    static cat::HCatBuffer SetMethod() { return "SetSystemVolume"; }
};
void to_xml(HXml &xml, const SystemVolumeInfo &data);
void from_xml(const HXml &xml, SystemVolumeInfo &data);


///< 设备名
struct DeviceNameInfo
{
    cat::HCatBuffer name;   ///< 设备名

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetDeviceName"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetDeviceName"; }
    static cat::HCatBuffer GetMethod() { return "GetDeviceName"; }
    static cat::HCatBuffer SetMethod() { return "SetDeviceName"; }
};
void to_xml(HXml &xml, const DeviceNameInfo &data);
void from_xml(const HXml &xml, DeviceNameInfo &data);


///< tcp服务器
struct SDKTcpServerInfo
{
    cat::HCatBuffer host;   ///< 主机地址
    huint16 port;           ///< 端口

    SDKTcpServerInfo() : port(0) {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetSDKTcpServer"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetSDKTcpServer"; }
    static cat::HCatBuffer GetMethod() { return "GetSDKTcpServer"; }
    static cat::HCatBuffer SetMethod() { return "SetSDKTcpServer"; }
};
void to_xml(HXml &xml, const SDKTcpServerInfo &data);
void from_xml(const HXml &xml, SDKTcpServerInfo &data);


///< 设备信息
struct DeviceInfo
{
    struct Device {
        cat::HCatBuffer cpu;    ///< cpu信息
        cat::HCatBuffer id;     ///< 设备id
        cat::HCatBuffer model;  ///< 设备类型
        cat::HCatBuffer name;   ///< 设备名
    };
    struct Version {
        cat::HCatBuffer kernel;     ///< 内核版本
        cat::HCatBuffer hardware;   ///< 硬件版本
        cat::HCatBuffer app;        ///< 应用版本
        cat::HCatBuffer fpga;       ///< fpga版本
    };
    struct Screen {
        int rotation;   ///< 旋转
        int width;      ///< 屏幕宽度
        int height;     ///< 屏幕高度
        Screen()
            : rotation(0)
            , width(0)
            , height(0)
        {}
    };

    Device device;
    Version version;
    Screen screen;

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetDeviceInfo"; }
    static cat::HCatBuffer GetMethod() { return "GetDeviceInfo"; }
};
void from_xml(const HXml &xml, DeviceInfo &data);


///< 屏幕截图
struct ScreenShot2
{
    int width;                  ///< 截图宽
    int height;                 ///< 截图高
    cat::HCatBuffer rawData;    ///< 图片base64数据

    ScreenShot2()
        : width(0)
        , height(0)
    {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetScreenshot2"; }
    static cat::HCatBuffer GetMethod() { return "GetScreenshot2"; }
};
void to_xml(HXml &xml, const ScreenShot2 &data);
void from_xml(const HXml &xml, ScreenShot2 &data);


///< 切换节目和获取当前节目guid
struct SwitchProgramInfo
{
    cat::HCatBuffer guid;
    int index;

    SwitchProgramInfo()
        : index(0)
    {}

    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SwitchProgram"; }
    static cat::HCatBuffer SetMethod() { return "SwitchProgram"; }
    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetCurrentPlayProgramGUID"; }
    static cat::HCatBuffer GetMethod() { return "GetCurrentPlayProgramGUID"; }
};
void to_xml(HXml &xml, const SwitchProgramInfo &data);
void from_xml(const HXml &xml, SwitchProgramInfo &data);


}


#endif // __HGENERALINFO_H__
