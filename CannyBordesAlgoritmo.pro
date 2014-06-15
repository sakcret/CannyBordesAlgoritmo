#-------------------------------------------------
#
# Project created by QtCreator 2014-06-06T14:39:03
#
#-------------------------------------------------

QT       += core gui

CONFIG+= console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CannyBordesAlgoritmo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    CImage.cpp

HEADERS  += mainwindow.h \
    CImage.h \
    CMatrix.h \
    globals.h

FORMS    += mainwindow.ui
