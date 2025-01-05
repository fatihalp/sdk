

#ifndef __HTIMEINFO_H__
#define __HTIMEINFO_H__


#include <HCatBuffer.h>
#include <HXml.h>


namespace sdk
{


///< 时间
struct TimeInfo
{
    struct Rf {
        bool enable;    ///< rf同步是否使能
        bool master;    ///< 主设备
        int channel;    ///< 通道
        Rf()
            : enable(false)
            , master(false)
            , channel(0)
            {};
    };

    cat::HCatBuffer timeZone;   ///< 时区字符串
    bool summer;                ///< 是否开启夏令时
    cat::HCatBuffer sync;       ///< 时间同步模式
    cat::HCatBuffer currTime;   ///< 当前控制卡时间 yyyy-mm-dd hh:MM:ss
    cat::HCatBuffer serverList; ///< 服务器列表, 逗号分隔
    Rf rf;                      ///< rf模块

    TimeInfo()
        : summer(false)
    {}
    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetTimeInfo"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetTimeInfo"; }
    static cat::HCatBuffer GetMethod() { return "GetTimeInfo"; }
    static cat::HCatBuffer SetMethod() { return "SetTimeInfo"; }
};
void from_xml(const HXml &xml, TimeInfo &node);
void to_xml(HXml &xml, const TimeInfo &node);


}

#endif // __HTIMEINFO_H__
