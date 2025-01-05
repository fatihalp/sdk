

#ifndef __HCatNet_TOOL_H__
#define __HCatNet_TOOL_H__


#include <limits>
#include <HCatNetInfo.h>
#include <string>


namespace HTool
{


template <typename _T>
std::string IconvStr(_T value) {
    std::string ret;
    for (std::size_t i = 0; i < sizeof(_T); ++i) {
        ret.push_back(char(value & 0xff));
        value = value >> 8;
    }
    return ret;
}

template <typename _T>
_T StrIconv(const char *value, std::size_t len = std::numeric_limits<std::size_t>::max()) {
    _T ret = _T();
    for (std::size_t i = 0; i < sizeof(_T) && i < len; ++i) {
        ret = ret | ((value[i] & 0xff) << (8 * i));
    }
    return ret;
}

template <typename _T>
_T StrIconv(const std::string &value) { return StrIconv<_T>(value.data(), value.size()); }


///< 兼容性错误CRC32校验, 用于错误性传感器crc校验
huint32 HCATNET_EXPORT SensorCrc32(const char *data, hint32 len);
huint32 HCATNET_EXPORT crc32(const char *data, hint32 len);

huint16 HCATNET_EXPORT ModbusCrc16(const char *buf, hint32 len);

///< 要求filePath是utf8编码, exit为true时是退出
std::string HCATNET_EXPORT MD5_U8(const std::string &filePath, bool *exit = nullptr);

///< 生成uuid
std::string HCATNET_EXPORT Uuid();

///< utf8转换成本地字符串
std::string HCATNET_EXPORT ToLocalString(const std::string &str);

///< 获取文件大小
hint64 HCATNET_EXPORT GetFileSize(const std::string &filePath);

}

#endif // __HCatNet_TOOL_H__
