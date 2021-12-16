ACSS_OUT_ROOT = $${OUT_PWD}/../..

QT += core gui widgets

CONFIG += c++14
CONFIG += debug_and_release

DESTDIR = $${ACSS_OUT_ROOT}/lib
TARGET = full_features
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
    
RESOURCES += full_features.qrc


LIBS += -L$${ACSS_OUT_ROOT}/lib
include(../../acss.pri)
INCLUDEPATH += ../../src
DEPENDPATH += ../../src  
