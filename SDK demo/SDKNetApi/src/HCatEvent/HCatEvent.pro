CONFIG -= qt

TEMPLATE = lib
CONFIG += static
DEFINES += HCATEVENT_LIBRARY

OUT_NAME = HCatEvent
contains(CONFIG, static) {
OUT_NAME = HCatEvent_static
}

win32:CONFIG(debug, debug|release): CONFIG(debug, debug|release): TARGET = $$OUT_NAME"d"
else:win32:CONFIG(release, debug|release): TARGET = $$OUT_NAME
else:unix:!macx: TARGET = $$OUT_NAME

contains(QT_ARCH, i386) {
    DESTDIR = $$PWD/../bin
} else {
    DESTDIR = $$PWD/../bin64
}


SOURCES += \
    HCatEventCore.cpp \
    HCatGcEvent.cpp \
    HCatIOEvent.cpp \
    HCatIOReadWriteEvent.cpp \
    HCatMainEvent.cpp \
    HCatNet/HCatSerial.cpp \
    HCatNet/HCatTcpService.cpp \
    HCatNet/HCatTcpSocket.cpp \
    HCatNet/HCatUdpSocket.cpp \
    HCatThreadTaskEvent.cpp \
    HCatTimeEvent.cpp \
    HCatTool.cpp \
    ICatEventApi.cpp \
    ICatSyncCoreEvent.cpp

HEADERS += \
    HCatAny.h \
    HCatBuffer.h \
    HCatEventCore.h \
    HCatEventInfo.h \
    HCatEventStruct.h \
    HCatForward.h \
    HCatGcEvent.h \
    HCatIOEvent.h \
    HCatIOReadWriteEvent.h \
    HCatMainEvent.h \
    HCatNet/HCatSerial.h \
    HCatNet/HCatTcpService.h \
    HCatNet/HCatTcpSocket.h \
    HCatNet/HCatUdpSocket.h \
    HCatSignal.h \
    HCatSignalSet.h \
    HCatThread.h \
    HCatThreadTaskEvent.h \
    HCatTimeEvent.h \
    HCatTimer.h \
    HCatTool.h \
    ICatEventApi.h \
    ICatEventBase.h \
    ICatEventData.h \
    ICatSyncCoreEvent.h

INCLUDEPATH += $$(OPENSSL_PATH)
DEPENDPATH += $$(OPENSSL_PATH)

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
exists($$(OPENSSL_LIB)/libssl_static.lib) {
    SSL_NAME = $$(OPENSSL_LIB)/libssl_static
    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$SSL_NAME".a"
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$SSL_NAME".a"
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$SSL_NAME".lib"
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$SSL_NAME".lib"
    else:unix:!macx: PRE_TARGETDEPS += $$DESTDIR/$$SSL_NAME".a"

}
exists($$(OPENSSL_LIB)/libcrypto_static.lib) {
    CRYPTO_NAME = $$(OPENSSL_LIB)/libcrypto_static
    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$CRYPTO_NAME".a"
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$CRYPTO_NAME".a"
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$CRYPTO_NAME".lib"
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$CRYPTO_NAME".lib"
    else:unix:!macx: PRE_TARGETDEPS += $$CRYPTO_NAME".a"
}

LIBS += -L$$OPENSSL_LIB/ -l$$SSL_NAME -l$$CRYPTO_NAME


SOURCES += \
    HCatOpenSSL.cpp \
    HCatNet/HCatTcpTlsSocket.cpp


HEADERS += \
    HCatOpenSSL.h \
    HCatNet/HCatTcpTlsSocket.h

}
