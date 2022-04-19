ACSS_OUT_ROOT = $${OUT_PWD}/..
CONFIG += c++14
CONFIG += debug_and_release
TARGET = $$qtLibraryTarget(qtadvancedcss)
DEFINES += QT_DEPRECATED_WARNINGS
TEMPLATE = lib
DESTDIR = $${ACSS_OUT_ROOT}/lib
QT += core gui widgets qml svg

!acssBuildStatic {
	CONFIG += shared
    DEFINES += ACSS_SHARED_EXPORT
}
acsBuildStatic {
	CONFIG += staticlib
    DEFINES += ACSS_STATIC
}

windows {
	# MinGW
	*-g++* {
		QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
	}
	# MSVC
	*-msvc* {
                QMAKE_CXXFLAGS += /utf-8
        }
}

#RESOURCES += ads.qrc

HEADERS += \
    acss_globals.h \
	QmlStyleUrlInterceptor.h \
	QtAdvancedStylesheet.h


SOURCES += \
	QmlStyleUrlInterceptor.cpp \
	QtAdvancedStylesheet.cpp


isEmpty(PREFIX){
	PREFIX=../installed
	warning("Install Prefix not set")
}

headers.path=$$PREFIX/include
headers.files=$$HEADERS
target.path=$$PREFIX/lib
INSTALLS += headers target

DISTFILES +=
