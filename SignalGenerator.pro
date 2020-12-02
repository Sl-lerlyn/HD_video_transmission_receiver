lessThan(QT_VERSION, 5.7):error("The SignalGenerator SDK sample requires at least Qt version 5.7")

QT       += core gui widgets

TARGET = SignalGenerator
TEMPLATE = app
CONFIG += c++11
INCLUDEPATH = ../../include
INCLUDEPATH += $$PWD/../../../../../../桌面/sxd/Agora_Native_SDK_for_Linux_x64_rel.v2.7.1.909_FULL_20200731_1130/lib/linux/agora_media_sdk/include
LIBS += -ldl
LIBS += -L$$PWD/../../../../../../桌面/sxd/Agora_Native_SDK_for_Linux_x64_rel.v2.7.1.909_FULL_20200731_1130/lib/linux/agora_media_sdk/ -lagora_rtc_sdk


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS 	=	SignalGenerator.h \
				SignalGeneratorEvents.h \
				com_ptr.h \
				DeckLinkDeviceDiscovery.h \
				DeckLinkOutputDevice.h \
                                DeckLinkOpenGLWidget.h \
                                ConnectToAgora.h \
                                utils/log.h \
                                common/sample_common.h \
                                common/sample_local_user_observer.h \
                                common/helper.h \
                                common/sample_connection_observer.h \
                                common/write_csvfile.h \
                                common/opt_parser.h \
                                common/sample_event.h \
                                common/switch_video_stream_base.h \
                                ProfileCallback.h

SOURCES 	= 	main.cpp \
				../../include/DeckLinkAPIDispatch.cpp \
				DeckLinkDeviceDiscovery.cpp \
				DeckLinkOutputDevice.cpp \
				DeckLinkOpenGLWidget.cpp \
				SignalGenerator.cpp \
                                ConnectToAgora.cpp \
                                common/sample_common.cpp \
                                common/sample_local_user_observer.cpp \
                                common/helper.cpp \
                                common/sample_connection_observer.cpp \
                                common/write_csvfile.cpp \
                                common/opt_parser.cpp \
                                common/sample_event.cpp \
				ProfileCallback.cpp

FORMS 		= 	SignalGenerator.ui

