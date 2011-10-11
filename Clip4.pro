#-------------------------------------------------
#
# Project created by QtCreator 2010-09-13T13:57:08
#
#-------------------------------------------------

QT       += core gui opengl xml svg webkit

TARGET = Clip
TEMPLATE = app

win32:CONFIG(debug, debug|release) {
     CONFIG += console
}

DEFFILE = $(OBJECTS_DIR)/defs.o
win32 {
  DEFFILE = $(OBJECTS_DIR)\defs.o
  PATH += C:/Program Files (x86)/TortoiseHg
  PATH += C:/Programme/TortoiseHG
  PATH += C:/Program Files/TortoiseHg
}
static {
  QMAKE_LFLAGS += -static-libgcc
  QTPLUGIN += qjpeg qgif qmng qtiff qico qsvg
  DEFINES += __STATIC__
}

defs.commands = -\$(DEL_FILE) $$DEFFILE
QMAKE_EXTRA_TARGETS += defs
PRE_TARGETDEPS += defs

CONFIG(debug, debug|release) {
  DEFINES += __DEBUG__ __SOURCEDIR__="\"\\\"$$PWD\\\"\""
}

DEFINES += __HG_REPRO_ID__="\\\"\$(shell hg -R \"$$PWD\" -q id)\\\""
DEFINES += __HG_REPRO_REV__="\\\"\$(shell hg -R \"$$PWD\" -q parent --template {rev})\\\""
DEFINES += __HG_REPRO_DATE__="\"\\\"\$(shell hg -R \"$$PWD\" -q parent --template \"{date|date}\")\\\"\""


QMAKE_CXXFLAGS += -std=gnu++0x
LIBS += -lboost_thread

# Eigen linear Algebra library
INCLUDEPATH += ../eigen

QMAKE_CXXFLAGS_DEBUG += -pg -W -Wpointer-arith -Wcast-qual -Wcast-align -Wmissing-declarations -Wredundant-decls
QMAKE_LFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_RELEASE -= -Wl,-s
#QMAKE_CXXFLAGS += -Wl,-pie -pie -gstabs -g3 -shared-libgcc
#QMAKE_LFLAGS += -Wl,-pie -pie -gstabs -g3 -shared-libgcc


SOURCES += main.cpp\
        ui/clip.cpp \
    config/colorbutton.cpp \
    config/colorconfigitem.cpp \
    config/configstore.cpp \
    core/crystal.cpp \
    core/diffractingstereoprojector.cpp \
    core/laueplaneprojector.cpp \
    core/projector.cpp \
    core/projectorfactory.cpp \
    core/reflection.cpp \
    core/spacegroup.cpp \
    core/spacegroupdata.cpp \
    core/stereoprojector.cpp \
    defs.cpp \
    image/beziercurve.cpp \
    image/basdataprovider.cpp \
    image/brukerprovider.cpp \
    image/dataprovider.cpp \
    image/dataproviderfactory.cpp \
    image/datascaler.cpp \
    image/datascalerfactory.cpp \
    image/imagedatastore.cpp \
    image/laueimage.cpp \
    image/mwdataprovider.cpp \
    image/qimagedataprovider.cpp \
    image/simplemonochromscaler.cpp \
    image/simplergbscaler.cpp \
    image/xyzdataprovider.cpp \
    indexing/candidategenerator.cpp \
    indexing/indexer.cpp \
    indexing/livemarkermodel.cpp \
    indexing/marker.cpp \
    indexing/solution.cpp \
    indexing/solutionmodel.cpp \
    refinement/fitobject.cpp \
    refinement/fitparameter.cpp \
    refinement/fitparametergroup.cpp \
    refinement/fitparametertreeitem.cpp \
    refinement/neldermead.cpp \
    refinement/neldermead_worker.cpp \
    tools/abstractmarkeritem.cpp \
    tools/circleitem.cpp \
    tools/colortextitem.cpp \
    tools/combolineedit.cpp \
    tools/cropmarker.cpp \
    tools/diagramgv.cpp \
    tools/histogramitem.cpp \
    tools/indexparser.cpp \
    tools/itemstore.cpp \
    tools/mat3D.cpp \
    tools/numberedit.cpp \
    tools/objectstore.cpp \
    tools/optimalrotation.cpp \
    tools/propagatinggraphicsobject.cpp \
    tools/resizeingtablewidget.cpp \
    tools/ruleritem.cpp \
    tools/rulermodel.cpp \
    tools/spotindicatorgraphicsitem.cpp \
    tools/spotitem.cpp \
    tools/tools.cpp \
    tools/vec3D.cpp \
    tools/webkittextobject.cpp \
    tools/xmllistiterators.cpp \
    tools/xmltools.cpp \
    tools/zipiterator.cpp \
    tools/zoneitem.cpp \
    ui/clipconfig.cpp \
    ui/contrastcurves.cpp \
    ui/crystaldisplay.cpp \
    ui/fitdisplay.cpp \
    ui/hkltool.cpp \
    ui/imagetoolbox.cpp \
    ui/indexdisplay.cpp \
    ui/laueplanecfg.cpp \
    ui/mouseinfodisplay.cpp \
    ui/printdialog.cpp \
    ui/projectiongraphicsview.cpp \
    ui/projectionplane.cpp \
    ui/reorient.cpp \
    ui/resolutioncalculator.cpp \
    ui/rotatecrystal.cpp \
    ui/sadeasteregg.cpp \
    ui/stereocfg.cpp \ 
    tools/threadrunner.cpp

HEADERS  += ui/clip.h \
    config/colorbutton.h \
    config/colorconfigitem.h \
    config/configstore.h \
    core/crystal.h \
    core/diffractingstereoprojector.h \
    core/laueplaneprojector.h \
    core/projector.h \
    core/projectorfactory.h \
    core/reflection.h \
    core/spacegroup.h \
    core/stereoprojector.h \
    defs.h \
    image/basdataprovider.h \
    image/beziercurve.h \
    image/brukerprovider.h \
    image/dataprovider.h \
    image/dataproviderfactory.h \
    image/datascaler.h \
    image/datascalerfactory.h \
    image/imagedatastore.h \
    image/laueimage.h \
    image/mwdataprovider.h \
    image/qimagedataprovider.h \
    image/simplemonochromscaler.h \
    image/simplergbscaler.h \
    image/xyzdataprovider.h \
    indexing/candidategenerator.h \
    indexing/indexer.h \
    indexing/livemarkermodel.h \
    indexing/marker.h \
    indexing/solution.h \
    indexing/solutionmodel.h \
    refinement/fitobject.h \
    refinement/fitparameter.h \
    refinement/fitparametergroup.h \
    refinement/fitparametertreeitem.h \
    refinement/neldermead.h \
    refinement/neldermead_worker.h \
    tools/abstractmarkeritem.h \
    tools/circleitem.h \
    tools/colortextitem.h \
    tools/combolineedit.h \
    tools/cropmarker.h \
    tools/debug.h \
    tools/diagramgv.h \
    tools/histogramitem.h \
    tools/indexparser.h \
    tools/init3D.h \
    tools/itemstore.h \
    tools/mat3D.h \
    tools/mousepositioninfo.h \
    tools/numberedit.h \
    tools/objectstore.h \
    tools/optimalrotation.h \
    tools/propagatinggraphicsobject.h \
    tools/resizeingtablewidget.h \
    tools/ruleritem.h \
    tools/rulermodel.h \
    tools/spotindicatorgraphicsitem.h \
    tools/spotitem.h \
    tools/tools.h \
    tools/vec3D.h \
    tools/webkittextobject.h \
    tools/xmllistiterators.h \
    tools/xmltools.h \
    tools/zipiterator.h \
    tools/zoneitem.h \
    ui/clipconfig.h \
    ui/contrastcurves.h \
    ui/crystaldisplay.h \
    ui/fitdisplay.h \
    ui/hkltool.h \
    ui/imagetoolbox.h \
    ui/indexdisplay.h \
    ui/laueplanecfg.h \
    ui/mouseinfodisplay.h \
    ui/printdialog.h \
    ui/projectiongraphicsview.h \
    ui/projectionplane.h \
    ui/reorient.h \
    ui/resolutioncalculator.h \
    ui/rotatecrystal.h \
    ui/sadeasteregg.h \
    ui/stereocfg.h \
    tools/threadrunner.h \
    config.h

FORMS    += ui/clip.ui \
    ui/clipconfig.ui \
    ui/contrastcurves.ui \
    ui/crystaldisplay.ui \
    ui/fitdisplay.ui \
    ui/hkltool.ui \
    ui/imagetoolbox.ui \
    ui/indexdisplay.ui \
    ui/laueplanecfg.ui \
    ui/mouseinfodisplay.ui \
    ui/printdialog.ui \
    ui/projectionplane.ui \
    ui/reorient.ui \
    ui/resolutioncalculator.ui \
    ui/rotatecrystal.ui \
    ui/sadeasteregg.ui \
    ui/stereocfg.ui

RESOURCES += resources/resources.qrc

win32 {
    RC_FILE = resources/clip.rc
}

OTHER_FILES += \
    README.txt \
    Resources/AboutMessage.txt \
    Resources/clip.rc \
    Resources/report.css \
    Resources/report_LauePlaneProjector.html \
    Resources/report_crystal.html \
    Resources/report_image.html \
    Todo.txt 
