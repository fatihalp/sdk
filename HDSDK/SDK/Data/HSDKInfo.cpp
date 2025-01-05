

#include <Data/HSDKInfo.h>
#include <HCatBuffer.h>
#include <HXml.h>


using namespace sdk;


template <typename _T,
         bool(*)(const cat::HCatBuffer &) = &_T::MatchGet,
         bool(*)(const cat::HCatBuffer &) = &_T::MatchSet>
bool UpdateNode(_T &obj, const cat::HCatBuffer &method, const HXml &xml) {
    if (_T::MatchSet(method)) {
        return true;
    }

    if (_T::MatchGet(method) == false) {
        return false;
    }

    return xml.get_to(obj);
}
template <typename _T,
         bool(*)(const cat::HCatBuffer &) = &_T::MatchGet>
bool UpdateGetNode(_T &obj, const cat::HCatBuffer &method, const HXml &xml) {
    if (_T::MatchGet(method) == false) {
        return false;
    }

    return xml.get_to(obj);
}


int HSDKInfo::ParseInfo(tinyxml2::XMLElement *outNode, const cat::HCatBuffer &method)
{
    if (outNode == nullptr) {
        return 0;
    }

    int result = 0;
    HXml xml(outNode);

#define UPDATE_NODE(obj) \
    if (UpdateNode(obj, method, xml)) { ++result; }
#define UPDATE_GET_NODE(obj) \
    if (UpdateGetNode(obj, method, xml)) { ++result; }

    UPDATE_NODE(ethInfo);
    UPDATE_NODE(wifiInfo);
    UPDATE_NODE(pppoeInfo);
    UPDATE_NODE(systemVolumeInfo);
    UPDATE_NODE(deviceNameInfo);
    UPDATE_NODE(lightInfo);
    UPDATE_NODE(tcpSercerInfo);
    UPDATE_NODE(switchTimeInfo);
    UPDATE_NODE(relayInfo);
    UPDATE_NODE(timeInfo);
    UPDATE_NODE(switchProgramInfo);

    UPDATE_GET_NODE(deviceInfo);
    UPDATE_GET_NODE(screenShot2);
    UPDATE_GET_NODE(sensorInfo);
    UPDATE_GET_NODE(gpsInfo);

    return result;
#undef UPDATE_GET_NODE
#undef UPDATE_NODE
}
