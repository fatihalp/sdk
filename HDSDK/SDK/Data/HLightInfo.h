

#ifndef __HLIGHTINFO_H__
#define __HLIGHTINFO_H__


#include <HCatBuffer.h>
#include <HXml.h>
#include <vector>


namespace sdk
{


///< 亮度
struct LightInfo
{
    enum eMode {
        kDefault,   ///< 默认
        kPloys,     ///< 自定义模式
        kSensor,    ///< 传感器模式
    };
    struct Sensor {
        hint16 min;     ///< 最小亮度等级
        hint16 max;     ///< 最大亮度等级
        hint16 time;    ///< 亮度调整间隔时间

        Sensor() : min(1), max(100), time(5) {}
    };
    struct Ploy {
        bool enable;            ///< 使能
        cat::HCatBuffer start;  ///< 开始时间 hh:mm:ss
        hint16 percent;         ///< 亮度等级

        Ploy() : enable(false), percent(0) {}
    };

    eMode mode;                     ///< 模式
    cat::HCatBuffer defaultValue;   ///< 默认模式的值[1, 100]
    Sensor sensor;                  ///< 传感器模式
    std::vector<Ploy> ployList;     ///< 自定义模式

    LightInfo()
        : mode(kDefault)
    {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetLuminancePloy"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetLuminancePloy"; }
    static cat::HCatBuffer GetMethod() { return "GetLuminancePloy"; }
    static cat::HCatBuffer SetMethod() { return "SetLuminancePloy"; }
};
void from_xml(const HXml &xml, LightInfo &node);
void to_xml(HXml &xml, const LightInfo &node);


}


#endif // __HLIGHTINFO_H__
