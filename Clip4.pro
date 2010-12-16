#-------------------------------------------------
#
# Project created by QtCreator 2010-09-13T13:57:08
#
#-------------------------------------------------

QT       += core gui opengl xml

TARGET = Clip
TEMPLATE = app

win32 {
        CONFIG += console
}

DEFFILE = $(OBJECTS_DIR)/defs.o
win32 {
  DEFFILE = $(OBJECTS_DIR)\defs.o
  PATH += C:/Program Files (x86)/TortoiseHg
  PATH += C:/Programme/TortoiseHG
}

defs.commands = -\$(DEL_FILE) $$DEFFILE
QMAKE_EXTRA_TARGETS += defs
PRE_TARGETDEPS += defs


DEFINES += __HG_REPRO_ID__="\\\"\$(shell hg -R \"$$PWD\" -q id)\\\""
DEFINES += __HG_REPRO_REV__="\\\"\$(shell hg -R \"$$PWD\" -q parent --template {rev})\\\""
DEFINES += __HG_REPRO_DATE__="\"\\\"\$(shell hg -R \"$$PWD\" -q parent --template \"{date|date}\")\\\"\""


QMAKE_CXXFLAGS += -std=gnu++0x -g -pg

QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS += -pg
#QMAKE_LFLAGS_RELEASE -= -Wl,-s


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
    ui/stereocfg.cpp \
    ui/laueplanecfg.cpp \
    tools/ruleritem.cpp \
    ui/resolutioncalculator.cpp \
    tools/rulermodel.cpp \
    tools/zoneitem.cpp \
    image/laueimage.cpp \
#    image/ImageTools.cpp \
    image/BezierCurve.cpp \
    ui/imagetoolbox.cpp \
    tools/diagramgv.cpp \
    defs.cpp \
    image/dataprovider.cpp \
    image/dataproviderfactory.cpp \
    image/qimagedataprovider.cpp \
    image/basdataprovider.cpp \
    image/datascaler.cpp \
    image/datascalerfactory.cpp \
    image/simplergbscaler.cpp \
    tools/cropmarker.cpp \
    tools/propagatinggraphicsobject.cpp \
    ui/contrastcurves.cpp \
    image/simplemonochromscaler.cpp \
    tools/histogramitem.cpp \
    ui/mouseinfodisplay.cpp \
    indexing/indexer.cpp \
    indexing/solution.cpp \
    tools/itemstore.cpp \
    tools/circleitem.cpp \
#    tools/projectionmapper.cpp \
    tools/spotindicatorgraphicsitem.cpp \
    ui/indexdisplay.cpp \
    indexing/markermodel.cpp \
    core/spacegroupdata.cpp \
    indexing/candidategenerator.cpp \
    indexing/solutionmodel.cpp \
    image/xyzdataprovider.cpp \
    ui/rotatecrystal.cpp

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
    tools/init3D.h \
    ui/stereocfg.h \
    ui/laueplanecfg.h \
    tools/ruleritem.h \
    ui/resolutioncalculator.h \
    tools/rulermodel.h \
    tools/zoneitem.h \
    image/laueimage.h \
    image/ImageTools.h \
    image/BezierCurve.h \
    ui/imagetoolbox.h \
    tools/diagramgv.h \
    tools/debug.h \
    image/dataprovider.h \
    image/dataproviderfactory.h \
    image/qimagedataprovider.h \
    image/basdataprovider.h \
    image/datascaler.h \
    image/datascalerfactory.h \
    image/simplergbscaler.h \
    tools/cropmarker.h \
    tools/propagatinggraphicsobject.h \
    ui/contrastcurves.h \
    image/simplemonochromscaler.h \
    tools/histogramitem.h \
    tools/mousepositioninfo.h \
    ui/mouseinfodisplay.h \
    indexing/indexer.h \
    indexing/solution.h \
    tools/itemstore.h \
    tools/circleitem.h \
#    tools/projectionmapper.h \
    tools/spotindicatorgraphicsitem.h \
    ui/indexdisplay.h \
    indexing/markermodel.h \
    indexing/candidategenerator.h \
    indexing/solutionmodel.h \
    image/xyzdataprovider.h \
    ui/rotatecrystal.h

FORMS    += ui/clip.ui \
    ui/crystaldisplay.ui \
    ui/projectionplane.ui \
    ui/stereocfg.ui \
    ui/laueplanecfg.ui \
    ui/resolutioncalculator.ui \
    ui/imagetoolbox.ui \
    ui/contrastcurves.ui \
    ui/mouseinfodisplay.ui \
    ui/indexdisplay.ui \
    ui/rotatecrystal.ui

RESOURCES += \
    resources/resources.qrc

win32 {
    RC_FILE = resources/icons/clip.rc
}

OTHER_FILES += \
    Todo.txt
