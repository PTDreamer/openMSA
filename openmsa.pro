#-------------------------------------------------
#
# Project created by QtCreator 2018-10-09T21:17:26
#
#-------------------------------------------------

QT       += core gui
target.path = /home/jose/code
INSTALLS += target
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT +=network

TARGET = openmsa
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    hardware/lmx2326.cpp \
    hardware/hardwaredevice.cpp \
    hardware/deviceparser.cpp \
    hardware/ad9850.cpp \
    hardware/controllers/slimusb.cpp \
    hardware/controllers/interface.cpp \
    hardware/controllers/usbdevice.cpp \
    hardware/controllers/simulator.cpp \
    hardware/genericadc.cpp \
    hardware/msa.cpp \
    shared/comprotocol.cpp

HEADERS  += mainwindow.h \
    hardware/lmx2326.h \
    global_defs.h \
    hardware/hardwaredevice.h \
    hardware/deviceparser.h \
    hardware/ad9850.h \
    hardware/controllers/slimusb.h \
    hardware/controllers/interface.h \
    hardware/controllers/usbdevice.h \
    hardware/controllers/simulator.h \
    hardware/genericadc.h \
    hardware/msa.h \
    shared/comprotocol.h

FORMS   += mainwindow.ui
LIBS	+= -L./lib -lusb-1.0

DISTFILES += \
    todo.txt
