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

defs.commands = -\$(DEL_FILE) $$DEFFILE
QMAKE_EXTRA_TARGETS += defs
PRE_TARGETDEPS += defs


CONFIG(debug, debug|release) {
  DEFINES += __SOURCEDIR__="\"\\\"$$PWD\\\"\""
}

DEFINES += __HG_REPRO_ID__="\\\"\$(shell hg -R \"$$PWD\" -q id)\\\""
DEFINES += __HG_REPRO_REV__="\\\"\$(shell hg -R \"$$PWD\" -q parent --template {rev})\\\""
DEFINES += __HG_REPRO_DATE__="\"\\\"\$(shell hg -R \"$$PWD\" -q parent --template \"{date|date}\")\\\"\""



QMAKE_CXXFLAGS += -std=gnu++0x

# Eigen linear Algebra library
INCLUDEPATH += ../eigen

QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_RELEASE -= -Wl,-s
#QMAKE_CXXFLAGS += -Wl,-pie -pie -gstabs -g3 -shared-libgcc
#QMAKE_LFLAGS += -Wl,-pie -pie -gstabs -g3 -shared-libgcc

SOURCES += main.cpp\
        ui/clip.cpp \
    ui/crystaldisplay.cpp \
    tools/vec3D.cpp \
    tools/mat3D.cpp \
    core/crystal.cpp \
    refinement/fitobject.cpp \
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
    image/BezierCurve.cpp \
    ui/imagetoolbox.cpp \
    tools/diagramgv.cpp \
    defs.cpp \
    image/dataprovider.cpp \
    image/dataproviderfactory.cpp \
    image/qimagedataprovider.cpp \
    image/basdataprovider.cpp \
    image/mwdataprovider.cpp \
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
    tools/spotindicatorgraphicsitem.cpp \
    ui/indexdisplay.cpp \
    core/spacegroupdata.cpp \
    indexing/candidategenerator.cpp \
    indexing/solutionmodel.cpp \
    image/xyzdataprovider.cpp \
    ui/rotatecrystal.cpp \
    ui/reorient.cpp \
    tools/indexparser.cpp \
    tools/tools.cpp \
    core/projectorfactory.cpp \
    tools/xmllistiterators.cpp \
    tools/xmltools.cpp \
    tools/zipiterator.cpp \
    indexing/livemarkermodel.cpp \
    tools/abstractmarkeritem.cpp \
    tools/spotitem.cpp \
    indexing/marker.cpp \
    ui/fitdisplay.cpp \
    refinement/neldermead.cpp \
    refinement/neldermead_worker.cpp \
    refinement/fitparameter.cpp \
    refinement/fitparametergroup.cpp \
    refinement/fitparametertreeitem.cpp \
    ui/hkltool.cpp \
    tools/resizeingtablewidget.cpp \
    image/brukerprovider.cpp \
    ui/clipconfig.cpp \
    config/configstore.cpp \
    config/colorbutton.cpp \
    config/colorconfigitem.cpp \
    ui/printdialog.cpp \
    tools/colortextitem.cpp \
    image/imagedatastore.cpp \
    ui/sadeasteregg.cpp \
    tools/combolineedit.cpp \
    tools/webkittextobject.cpp \
    core/diffractingstereoprojector.cpp

HEADERS  += ui/clip.h \
    ui/crystaldisplay.h \
    tools/vec3D.h \
    tools/mat3D.h \
    defs.h \
    core/crystal.h \
    refinement/fitobject.h \
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
    image/BezierCurve.h \
    ui/imagetoolbox.h \
    tools/diagramgv.h \
    tools/debug.h \
    image/dataprovider.h \
    image/dataproviderfactory.h \
    image/qimagedataprovider.h \
    image/basdataprovider.h \
    image/mwdataprovider.h \
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
    tools/spotindicatorgraphicsitem.h \
    ui/indexdisplay.h \
    indexing/candidategenerator.h \
    indexing/solutionmodel.h \
    image/xyzdataprovider.h \
    ui/rotatecrystal.h \
    ui/reorient.h \
    tools/indexparser.h \
    tools/tools.h \
    core/projectorfactory.h \
    tools/xmllistiterators.h \
    tools/xmltools.h \
    tools/zipiterator.h \
    indexing/livemarkermodel.h \
    tools/abstractmarkeritem.h \
    tools/spotitem.h \
    indexing/marker.h \
    ui/fitdisplay.h \
    refinement/neldermead.h \
    refinement/neldermead_worker.h \
    refinement/fitparameter.h \
    refinement/fitparametergroup.h \
    refinement/fitparametertreeitem.h \
    ui/hkltool.h \
    tools/resizeingtablewidget.h \
    image/brukerprovider.h \
    ui/clipconfig.h \
    config/configstore.h \
    config/colorbutton.h \
    config/colorconfigitem.h \
    ui/printdialog.h \
    tools/colortextitem.h \
    image/imagedatastore.h \
    ui/sadeasteregg.h \
    tools/combolineedit.h \
    tools/webkittextobject.h \
    core/diffractingstereoprojector.h

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
    ui/rotatecrystal.ui \
    ui/reorient.ui \
    ui/fitdisplay.ui \
    ui/hkltool.ui \
    ui/clipconfig.ui \
    ui/printdialog.ui \
    ui/sadeasteregg.ui

RESOURCES += \
    resources/resources.qrc

win32 {
    RC_FILE = resources/clip.rc
}

OTHER_FILES += \
    Todo.txt \
    Resources/clip.rc \
    Resources/AboutMessage.txt \
    README.txt \
    Resources/report_crystal.html \
    Resources/report.css \
    Resources/report_LauePlaneProjector.html \
    Resources/report_image.html
