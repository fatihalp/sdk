

# 提供直接合并到项目, 而不是生成库的版本

INCLUDEPATH += $$PWD/../HCatEvent
DEPENDPATH += $$PWD/../HCatEvent
INCLUDEPATH += $$PWD/../HCatEvent/fmt/include
DEPENDPATH += $$PWD/../HCatEvent/fmt/include
INCLUDEPATH += $$PWD/../HCatEvent/fmt/src
DEPENDPATH += $$PWD/../HCatEvent/fmt/src

SOURCES += \
    $$PWD/../HCatEvent/HCatEventCore.cpp \
    $$PWD/../HCatEvent/HCatGcEvent.cpp \
    $$PWD/../HCatEvent/HCatIOEvent.cpp \
    $$PWD/../HCatEvent/HCatIOReadWriteEvent.cpp \
    $$PWD/../HCatEvent/HCatMainEvent.cpp \
    $$PWD/../HCatEvent/HCatNet/HCatSerial.cpp \
    $$PWD/../HCatEvent/HCatNet/HCatTcpService.cpp \
    $$PWD/../HCatEvent/HCatNet/HCatTcpSocket.cpp \
    $$PWD/../HCatEvent/HCatNet/HCatUdpSocket.cpp \
    $$PWD/../HCatEvent/HCatThreadTaskEvent.cpp \
    $$PWD/../HCatEvent/HCatTimeEvent.cpp \
    $$PWD/../HCatEvent/HCatTool.cpp \
    $$PWD/../HCatEvent/ICatSyncCoreEvent.cpp \
    $$PWD/../HCatEvent/ICatEventApi.cpp

HEADERS += \
    $$PWD/../HCatEvent/CatString.h \
    $$PWD/../HCatEvent/HCatAny.h \
    $$PWD/../HCatEvent/HCatBuffer.h \
    $$PWD/../HCatEvent/HCatEventCore.h \
    $$PWD/../HCatEvent/HCatEventInfo.h \
    $$PWD/../HCatEvent/HCatEventStruct.h \
    $$PWD/../HCatEvent/HCatForward.h \
    $$PWD/../HCatEvent/HCatGcEvent.h \
    $$PWD/../HCatEvent/HCatIOEvent.h \
    $$PWD/../HCatEvent/HCatIOReadWriteEvent.h \
    $$PWD/../HCatEvent/HCatMainEvent.h \
    $$PWD/../HCatEvent/HCatNet/HCatSerial.h \
    $$PWD/../HCatEvent/HCatNet/HCatTcpService.h \
    $$PWD/../HCatEvent/HCatNet/HCatTcpSocket.h \
    $$PWD/../HCatEvent/HCatNet/HCatUdpSocket.h \
    $$PWD/../HCatEvent/HCatSignal.h \
    $$PWD/../HCatEvent/HCatSignalSet.h \
    $$PWD/../HCatEvent/HCatThread.h \
    $$PWD/../HCatEvent/HCatThreadTaskEvent.h \
    $$PWD/../HCatEvent/HCatTimeEvent.h \
    $$PWD/../HCatEvent/HCatTimer.h \
    $$PWD/../HCatEvent/HCatTool.h \
    $$PWD/../HCatEvent/ICatEventApi.h \
    $$PWD/../HCatEvent/ICatEventBase.h \
    $$PWD/../HCatEvent/ICatSyncCoreEvent.h \
    $$PWD/../HCatEvent/ICatEventData.h
    
