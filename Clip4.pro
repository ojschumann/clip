#-------------------------------------------------
#
# Project created by QtCreator 2010-09-13T13:57:08
#
#-------------------------------------------------

QT       += core gui

TARGET = Clip4
TEMPLATE = app


SOURCES += main.cpp\
        ui/clip.cpp \
    ui/crystaldisplay.cpp \
    core/vec3D.cpp \
    core/mat3D.cpp \
    core/crystal.cpp \
    core/fitobject.cpp \
    core/projector.cpp \
    core/objectstore.cpp \
    core/reflection.cpp \
    core/spacegroup.cpp

HEADERS  += ui/clip.h \
    ui/crystaldisplay.h \
    core/vec3D.h \
    core/mat3D.h \
    defs.h \
    core/crystal.h \
    core/fitobject.h \
    core/projector.h \
    core/objectstore.h \
    core/reflection.h \
    core/spacegroup.h

FORMS    += ui/clip.ui \
    ui/crystaldisplay.ui

RESOURCES += \
    icons/icons.qrc
