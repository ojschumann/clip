#-------------------------------------------------
#
# Project created by QtCreator 2010-09-13T13:57:08
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = Clip
TEMPLATE = app

win32 {
        CONFIG += console
}

HG = 'C:\\Programme\\TortoiseHG\\hg.exe'
HGID = $$SYSTEM($$HG -q id)
HGREV = $$SYSTEM($$HG -q parent --template {rev})
HGDATE = $$SYSTEM($$HG -q parent --template \"{date|date}\")

HGID_CSTR = $$sprintf("const char* HG_REPRO_ID = \"%1\";", $$HGID)
HGREV_CSTR = $$sprintf("const char* HG_REPRO_REV = \"%1\";", $$HGREV)
HGDATE_CSTR = $$sprintf("const char* HG_REPRO_DATE = \"%1\";", $$HGDATE)

hg.cpp.target = hg.cpp
hg.cpp.commands = @echo "$$HGID_CSTR $$HGREV_CSTR $$HGDATE_CSTR" > defs.cpp
hg.cpp.depends = lala

lala.commands = @echo $$HGDATE


QMAKE_EXTRA_TARGETS += hg.cpp lala
PRE_TARGETDEPS += hg.cpp

QMAKE_CXXFLAGS += -std=gnu++0x

QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg

SOURCES += main.cpp\
        ui/clip.cpp \
    ui/crystaldisplay.cpp \
    tools/vec3D.cpp \
    tools/mat3D.cpp \
    core/crystal.cpp \
    core/fitobject.cpp \
    core/projector.cpp \
    tools/objectstore.cpp \
    core/reflection.cpp \
    core/spacegroup.cpp \
    tools/optimalrotation.cpp \
    tools/numberedit.cpp \
    ui/projectionplane.cpp \
    ui/projectiongraphicsview.cpp \
    core/laueplaneprojector.cpp \
    core/stereoprojector.cpp \
    tools/signalingellipse.cpp \
    ui/stereocfg.cpp \
    ui/laueplanecfg.cpp \
    tools/ruleritem.cpp \
    ui/resolutioncalculator.cpp \
    tools/rulermodel.cpp \
    tools/zoneitem.cpp \
    defs.cpp

HEADERS  += ui/clip.h \
    ui/crystaldisplay.h \
    tools/vec3D.h \
    tools/mat3D.h \
    defs.h \
    core/crystal.h \
    core/fitobject.h \
    core/projector.h \
    tools/objectstore.h \
    core/reflection.h \
    core/spacegroup.h \
    tools/optimalrotation.h \
    tools/numberedit.h \
    ui/projectionplane.h \
    ui/projectiongraphicsview.h \
    core/laueplaneprojector.h \
    core/stereoprojector.h \
    tools/signalingellipse.h \
    tools/init3D.h \
    ui/stereocfg.h \
    ui/laueplanecfg.h \
    tools/ruleritem.h \
    ui/resolutioncalculator.h \
    tools/rulermodel.h \
    tools/zoneitem.h

FORMS    += ui/clip.ui \
    ui/crystaldisplay.ui \
    ui/projectionplane.ui \
    ui/stereocfg.ui \
    ui/laueplanecfg.ui \
    ui/resolutioncalculator.ui

RESOURCES += \
    icons/icons.qrc

win32 {
    RC_FILE = icons/clip.rc
}

OTHER_FILES += \
    Todo.txt
