

# 提供事件接口所需的头文件和一些头文件库

include($$PWD/../HCatEvent/HCatApi.pri)

HEADERS += \
    $$PWD/../HCatNet/ISession.h \
    $$PWD/../HCatNet/HCatSyncCoreEvent.h \
    $$PWD/../HCatNet/HCatEventToQtEvent.h \
    $$PWD/../HCatNet/HCatNetInfo.h \
    $$PWD/../HCatNet/IEventApi.h

INCLUDEPATH += $$PWD/../HCatEvent/
INCLUDEPATH += $$PWD/../HCatNet/
