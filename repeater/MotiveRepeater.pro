#-------------------------------------------------
#
# Project created by QtCreator 2016-01-07T15:12:45
#
#-------------------------------------------------


#NatNet
NATNETDIR=H:/cpp/natnet
INCLUDEPATH += src $${NATNETDIR}/include ../../mayaThreadedDevice
LIBS += -L$${NATNETDIR}/lib/x64 -lNatNetLibStatic

#Core
INCLUDEPATH += ../../core

LIBS += -lWs2_32 -lsetupapi

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MotiveRepeater
TEMPLATE = app

CFLAGS += /fp:strict

SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp \
    ../../core/motive.cpp \
    ../../core/rigidbody.cpp \
    ../../core/interfacecombo.cpp \
    ../../core/tcpconnector.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    ../../core/motive.h \
    ../../core/rigidbody.h \
    ../../core/interfacecombo.h \
    ../../mayaThreadedDevice/item.h \
    ../../core/tcpconnector.h

FORMS    += mainwindow.ui \
    settingsdialog.ui
