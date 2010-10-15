#-------------------------------------------------
#
# Project created by QtCreator 2010-09-13T13:57:08
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = Clip
TEMPLATE = app

QMAKE_CXXFLAGS += -std=gnu++0x

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
    tools/optimalrotation.cpp \
    tools/numberedit.cpp \
    ui/projectionplane.cpp \
    ui/projectiongraphicsview.cpp \
    core/laueplaneprojector.cpp \
    core/stereoprojector.cpp \
    tools/signalingellipse.cpp

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
    tools/optimalrotation.h \
    tools/numberedit.h \
    ui/projectionplane.h \
    ui/projectiongraphicsview.h \
    core/laueplaneprojector.h \
    core/stereoprojector.h \
    tools/signalingellipse.h \
    tools/init3D.h

FORMS    += ui/clip.ui \
    ui/crystaldisplay.ui \
    ui/projectionplane.ui

RESOURCES += \
    icons/icons.qrc

win32 {
    RC_FILE = icons/clip.rc
}
