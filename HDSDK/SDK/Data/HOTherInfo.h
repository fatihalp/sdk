

#ifndef __HOTHERINFO_H__
#define __HOTHERINFO_H__


#include <HCatBuffer.h>
#include <HXml.h>


namespace sdk
{


///< 多屏同步
struct MulScreenSyncInfo
{
    bool enable;    ///< 使能

    MulScreenSyncInfo() : enable(false) {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetMulScreenSync"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetMulScreenSync"; }
    static cat::HCatBuffer GetMethod() { return "GetMulScreenSync"; }
    static cat::HCatBuffer SetMethod() { return "SetMulScreenSync"; }
};
void from_xml(const HXml &xml, MulScreenSyncInfo &node);
void to_xml(HXml &xml, const MulScreenSyncInfo &node);


///< gps数据上报
struct GpsRespondInfo
{
    bool enable;
    hint16 delay;

    GpsRespondInfo() : enable(false), delay(0) {}

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetGpsRespondEnable"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetGpsRespondEnable"; }
    static cat::HCatBuffer GetMethod() { return "GetGpsRespondEnable"; }
    static cat::HCatBuffer SetMethod() { return "SetGpsRespondEnable"; }
};
void from_xml(const HXml &xml, GpsRespondInfo &node);
void to_xml(HXml &xml, const GpsRespondInfo &node);


}


#endif // __HOTHERINFO_H__
