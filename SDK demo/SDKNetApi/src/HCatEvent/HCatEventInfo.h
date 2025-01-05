#ifndef HCATEVENTINFO_H
#define HCATEVENTINFO_H


#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define H_WIN
#  define H_CATEVENT_EXPORT __declspec(dllexport)
#  define H_CATEVENT_IMPORT __declspec(dllimport)
#else
#define H_LINUX
#  define H_CATEVENT_EXPORT     __attribute__((visibility("default")))
#  define H_CATEVENT_IMPORT     __attribute__((visibility("default")))
#endif

// 使用静态库时, 不需要导出文件
#ifdef STATIC_IMPORT
#ifdef H_CATEVENT_IMPORT
#undef H_CATEVENT_IMPORT
#define H_CATEVENT_IMPORT
#endif
#endif

#if defined(HCATEVENT_LIBRARY)
#  define HCATEVENT_EXPORT H_CATEVENT_EXPORT
#else
#  define HCATEVENT_EXPORT H_CATEVENT_IMPORT
#endif


typedef unsigned long long      huint64;
typedef unsigned long           huint32l;
typedef unsigned int            huint32;
typedef unsigned short          huint16;
typedef unsigned char           huint8;

typedef long long               hint64;
typedef long                    hint32l;
typedef int                     hint32;
typedef short                   hint16;
typedef char                    hint8;
typedef float                   hfloat;
typedef double                  hdouble;


#ifndef UNUSED
# define UNUSED(x) (void)(x)
#endif


#endif // HCATEVENTINFO_H
