CONFIG -= qt

TEMPLATE = lib
#CONFIG += static
DEFINES += HCATNET_LIBRARY

# 这个是会话Debug信号日志输出
DEFINES += DEBUG_LOG_SIGNAL

OUT_NAME = HCatNet
contains(CONFIG, static) {
OUT_NAME = HCatNet_static
}

win32:CONFIG(debug, debug|release): CONFIG(debug, debug|release): TARGET = $$OUT_NAME"d"
else:win32:CONFIG(release, debug|release): TARGET = $$OUT_NAME
else:unix:!macx: TARGET = $$OUT_NAME
OUT_NAME = $$TARGET

contains(QT_ARCH, i386) {
    DESTDIR = $$PWD/../bin
} else {
    DESTDIR = $$PWD/../bin64
}

SOURCES += \
    HCApiImpl.cpp \
    HCatSyncCoreEvent.cpp \
    ICApi.cpp \
    IEventApi.cpp \
    ISession.cpp \
    Net/HSerialNet.cpp \
    Net/HTcpNet.cpp \
    Net/HTcpServiceNet.cpp \
    Net/HUdpNet.cpp \
    Protocol/HHexProtocol.cpp \
    Protocol/HHttpProtocol.cpp \
    Protocol/HLcdSdkProtocol.cpp \
    Protocol/HOldSDKProtocol.cpp \
    Protocol/HRawStringProtocol.cpp \
    Protocol/HSDKProtocol.cpp \
    Protocol/HSerialSDKProtocol.cpp \
    Protocol/HStruct.cpp \
    Protocol/HUdpFindDeviceProtocol.cpp \
    Tool/HTool.cpp \
    Tool/md5.cpp \
    Tool/tinyxml2.cpp

HEADERS += \
    CatString.h \
    HCApiImpl.h \
    HCatEventToQtEvent.h \
    HCatNetInfo.h \
    HCatSyncCoreEvent.h \
    HQtThread.h \
    ICApi.h \
    ICApiStatic.h \
    IEventApi.h \
    ISession.h \
    Net/HSerialNet.h \
    Net/HTcpNet.h \
    Net/HTcpServiceNet.h \
    Net/HUdpNet.h \
    Net/INetBase.h \
    Protocol/HHexProtocol.h \
    Protocol/HHttpProtocol.h \
    Protocol/HLcdSdkProtocol.h \
    Protocol/HOldSDKProtocol.h \
    Protocol/HRawStringProtocol.h \
    Protocol/HSDKProtocol.h \
    Protocol/HSerialSDKProtocol.h \
    Protocol/HStruct.h \
    Protocol/HUdpFindDeviceProtocol.h \
    Protocol/IProtocolBase.h \
    Tool/HTool.h \
    Tool/md5.h \
    Tool/tinyxml2.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += $$PWD/../HCatEvent
DEPENDPATH += $$PWD/../HCatEvent

INCLUDEPATH += $$PWD/../HCatEvent/fmt/include
DEPENDPATH += $$PWD/../HCatEvent/fmt/include
INCLUDEPATH += $$PWD/../HCatEvent/fmt/src
DEPENDPATH += $$PWD/../HCatEvent/fmt/src

OPENSSL_LIB_DEFIND = $$(OPENSSL_LIB)

# 判断OpenSSL的库路径环境变量是否存在
contains(DEFINES, OPENSSL_ENABLE_DEFINE) {
    !isEmpty(OPENSSL_LIB_DEFIND) {
        exists($$(OPENSSL_LIB)) {
            DEFINES += OPEN_SSL
        }
    }
}

contains(DEFINES, OPEN_SSL) {
SSL_NAME = $$(OPENSSL_LIB)/libssl
CRYPTO_NAME = $$(OPENSSL_LIB)/libcrypto
LIBS += -L$$(OPENSSL_LIB)/ -l$$SSL_NAME -l$$CRYPTO_NAME

SOURCES += \
    Net/HTcpTlsNet.cpp \

HEADERS += \
    Net/HTcpTlsNet.h \

}


contains(CONFIG, static) {
    DEFINES += HCATEVENT_LIBRARY
    include($$PWD/../HCatEvent/HCatEvent.pri)
} else {
    HCATEVENT_NAME = HCatEvent
    HCATEVENT_LIB_NAME = HCatEvent
    exists($$DESTDIR/HCatEvent_staticd.lib) {
        CONFIG(debug, debug|release) : HCATEVENT_NAME = HCatEvent_staticd
        CONFIG(debug, debug|release) : HCATEVENT_LIB_NAME = HCatEvent_staticd
        CONFIG(debug, debug|release) : DEFINES += STATIC_IMPORT
    }
    exists($$DESTDIR/HCatEvent_static.lib) {
        CONFIG(release, debug|release) : HCATEVENT_NAME = HCatEvent_static
        CONFIG(release, debug|release) : HCATEVENT_LIB_NAME = HCatEvent_static
        CONFIG(release, debug|release) : DEFINES += STATIC_IMPORT
    }

    exists($$DESTDIR/libHCatEvent_static.a) {
        HCATEVENT_NAME = libHCatEvent_static
        HCATEVENT_LIB_NAME = HCatEvent_static
        DEFINES += STATIC_IMPORT
    }

    win32:CONFIG(release, debug|release): LIBS += -L$$DESTDIR/ -l$$HCATEVENT_LIB_NAME
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$DESTDIR/ -l$$HCATEVENT_LIB_NAME
    else:unix:!macx: LIBS += -L$$DESTDIR/ -l$$HCATEVENT_LIB_NAME

    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".a"
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".a"
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".lib"
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".lib"
    else:unix:!macx: PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".a"
}

# 如果不存在uuid库, 可以增加全局宏不使用uuid库
unix:!RK_ANDROID {
    !contains(DEFINES, RK_ANDROID) {
        LIBS += -luuid
    }
}
