#-------------------------------------------------
#
# Project created by QtCreator 2011-01-20T13:37:10
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_clipunittesttest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_clipunittesttest.cpp \
           ../tools/mat3d.cpp

QMAKE_CXXFLAGS += -I.. -I../..
QMAKE_CXXFLAGS += -std=gnu++0x

