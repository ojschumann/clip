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
    tools/vec3D.cpp \
    tools/mat3D.cpp \
    core/crystal.cpp \
    core/fitobject.cpp \
    core/projector.cpp \
    core/objectstore.cpp \
    core/reflection.cpp \
    core/spacegroup.cpp \
    tools/optimalrotation.cpp

HEADERS  += ui/clip.h \
    ui/crystaldisplay.h \
    tools/vec3D.h \
    tools/mat3D.h \
    defs.h \
    core/crystal.h \
    core/fitobject.h \
    core/projector.h \
    core/objectstore.h \
    core/reflection.h \
    core/spacegroup.h \
    tools/optimalrotation.h

FORMS    += ui/clip.ui \
    ui/crystaldisplay.ui

RESOURCES += \
    icons/icons.qrc
