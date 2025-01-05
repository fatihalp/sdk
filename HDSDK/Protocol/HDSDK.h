

#ifndef __HDSDK_H__
#define __HDSDK_H__


#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define H_DECL_EXPORT __declspec(dllexport)
#  define H_DECL_IMPORT __declspec(dllimport)
#  define DLL_CALL      __cdecl
#  define H_WIN
#else
#  define H_DECL_EXPORT     __attribute__((visibility("default")))
#  define H_DECL_IMPORT     __attribute__((visibility("default")))
#  define DLL_CALL
// __attribute__((__cdecl__))
#  define H_LINUX
#endif

#ifdef STATIC_IMPORT
#ifdef H_DECL_IMPORT
#undef H_DECL_IMPORT
#define H_DECL_IMPORT
#endif
#endif

#if defined(USE_HD_LIB)
#  define HD_API H_DECL_EXPORT
#else
#  define HD_API H_DECL_IMPORT
#endif


#if (defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)) && !defined(HCAT_NOEXCEPTION)
#define CAT_THROW(exception) throw exception
#define CAT_TRY try
#define CAT_CATCH(exception) catch(exception)
#define CAT_INTERNAL_CATCH(exception) catch(exception)
#else
#include <cstdlib>
#define CAT_THROW(exception) std::abort()
#define CAT_TRY if(true)
#define CAT_CATCH(exception) if(false)
#define CAT_INTERNAL_CATCH(exception) if(false)
#endif


// override exception macros
#if defined(CAT_THROW_USER)
#undef CAT_THROW
#define CAT_THROW CAT_THROW_USER
#endif
#if defined(CAT_TRY_USER)
#undef CAT_TRY
#define CAT_TRY CAT_TRY_USER
#endif
#if defined(CAT_CATCH_USER)
#undef CAT_CATCH
#define CAT_CATCH CAT_CATCH_USER
#undef CAT_INTERNAL_CATCH
#define CAT_INTERNAL_CATCH CAT_CATCH_USER
#endif
#if defined(CAT_INTERNAL_CATCH_USER)
#undef CAT_INTERNAL_CATCH
#define CAT_INTERNAL_CATCH CAT_INTERNAL_CATCH_USER
#endif


#define HBool   int
#define HTrue   1
#define HFalse  0

typedef unsigned long long      huint64;
typedef unsigned int            huint32;
typedef unsigned short          huint16;
typedef unsigned char           huint8;

typedef long long               hint64;
typedef int                     hint32;
typedef short                   hint16;
typedef char                    hint8;


#ifdef USE_HD_LIB
namespace hd {
typedef class HDSDKPrivate* IHDProtocol;
}
using hd::IHDProtocol;
#else
typedef void* IHDProtocol;
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    /**
     * void readXml(const char *xml, int len, int errorCode, void *userData)
     * @param xml Xml data
     * @param len Xml len
     * @param errorCode Error code, normally 0
     * @param userData kSetReadXmlData value
     */
    kSetReadXml         = 0x0001,
    kSetReadXmlData     = 0x0002,

    /**
     * HBool sendDataToNet(const char *data, int len, void *userData)
     * @param data Send data
     * @param len Send data len
     * @param userData kSetSendFuncData value
     * @return 0 <= return. Stop the protocol, which requires a protocol reset
     */
    kSetSendFunc        = 0x0003,
    kSetSendFuncData    = 0x0004

} eHDFunc ;


HD_API const char * DLL_CALL Version();

/**
 * @brief CreateProtocol Create protocol. default init.
 * @return IHDProtocol
 */
HD_API IHDProtocol DLL_CALL CreateProtocol();

/**
 * @brief FreeProtocol free protocol
 * @param protocol IHDProtocol
 */
HD_API void DLL_CALL FreeProtocol(IHDProtocol protocol);

/**
 * @brief SetProtocolFunc Set protocol function
 * @param protocol IHDProtocol
 * @param func eHDFunc
 * @param data data
 * @return 1 or 0 (1 = true; 0 = false)
 */
HD_API HBool DLL_CALL SetProtocolFunc(IHDProtocol protocol, int func, void *data);

/**
 * @brief InitProtocol init protocol. Required when the same protocol body needs to be reused
 * @param protocol IHDProtocol
 */
HD_API void DLL_CALL InitProtocol(IHDProtocol protocol);

/**
 * @brief RunProtocol Start negotiating the protocol
 * @param protocol IHDProtocol
 * @return 1 or 0 (1 = true; 0 = false)
 */
HD_API HBool RunProtocol(IHDProtocol protocol);

/**
 * @brief UpdateReadData The core, the read data is passed in
 * @param protocol IHDProtocol
 * @param data Read data
 * @param len Read data len
 * @return 1 or 0 (1 = true; 0 = false)
 */
HD_API HBool UpdateReadData(IHDProtocol protocol, const char *data, int len);

/**
 * @brief SendXml Send xml data
 * @param protocol IHDProtocol
 * @param xml Xml data
 * @param len Xml len
 * @return 1 or 0 (1 = true; 0 = false)
 */
HD_API HBool DLL_CALL SendXml(IHDProtocol protocol, const char *xml, int len);

/**
 * @brief SendFile Send file
 * @param protocol IHDProtocol
 * @param fileName device save file name
 * @param fileNameLen Send file name len
 * @param md5 md5(32byte)
 * @param type 0(image), 1(video), 2(font), 3(firmware), 128(temp image file), 129(temp video file)
 * @param size Send file Size
 * @param callFun Send file data { int(hint64 index, hint64 size, int status, char *buff, int buffSize, void *userData) }
 * @param userData callFun userData
 * @return 1 or 0 (1 = true; 0 = false)
 */
HD_API HBool DLL_CALL SendFile(IHDProtocol protocol, const char *fileName, int fileNameLen, const char *md5, int type, hint64 size, int(*callFun)(hint64, hint64, int, char *, int, void *), void *userData);


#ifdef __cplusplus
}
#endif

#endif // __HDSDK_H__
