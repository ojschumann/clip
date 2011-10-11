/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#include "crystaldisplay.h"
#include "ui_crystaldisplay.h"

#include <QTableWidgetItem>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QXmlStreamWriter>
#include <QSettings>
#include <QMessageBox>

#include "tools/mat3D.h"
#include "core/crystal.h"
#include "core/spacegroup.h"
#include "ui/hkltool.h"
#include "ui/clip.h"
#include "tools/tools.h"
#include "tools/xmltools.h"

CrystalDisplay::CrystalDisplay(QWidget* _parent) :
    QMainWindow(_parent),
    ui(new Ui::CrystalDisplay), crystal(new Crystal(this)), allowRotationUpdate(true)
{
  ui->setupUi(this);

  // This is an interactive Crystal, enable background generation of reflections
  crystal->synchronUpdate(false);

  // Connect Signals of the crystal
  connect(crystal, SIGNAL(orientationChanged()), this, SLOT(slotUpdateOrientationMatrix()));
  connect(crystal, SIGNAL(cellChanged()), this, SLOT(slotUpdateOrientationMatrix()));
  connect(crystal, SIGNAL(cellChanged()), this, SLOT(slotLoadCellFromCrystal()));

  //connect(crystal, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
  connect(crystal, SIGNAL(info(QString, int)), this, SIGNAL(info(QString, int)));

  QWidget* spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // toolBar is a pointer to an existing toolbar
  ui->toolBar->insertWidget(ui->actionDrag, spacer);

  connect(crystal->getSpacegroup(), SIGNAL(constrainsChanged()), this, SLOT(slotSetSGConstrains()));

  ui->orientationMatrix->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->orientationMatrix->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

  // Load Orientation Matrix
  slotUpdateOrientationMatrix();

  // Load Cell
  slotLoadCellFromCrystal();

  // Disable Inputs based on constrains
  slotSetSGConstrains();

}

CrystalDisplay::~CrystalDisplay()
{
  delete ui;
  delete crystal;
}


QSize CrystalDisplay::sizeHint() const {
  // returning minimumSizeHint results in a compact window
  return minimumSizeHint();
}

void CrystalDisplay::slotUpdateOrientationMatrix() {
  // Load rotationMatrix
  Mat3D OM = crystal->getRotationMatrix();
  // Multiply with unrotated Orientation Matrix
  OM *= crystal->getReziprocalOrientationMatrix();
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      // Set Cells
      ui->orientationMatrix->item(i,j)->setText(QString::number(OM(i,j), 'f', 4));
    }
  }
  // let Crystal calc Euler angles and write to UI
  QList<double> euler = crystal->calcEulerAngles(true);
  allowRotationUpdate = false;
  ui->rotationOmega->setValue(euler[0]);
  ui->rotationChi->setValue(euler[1]);
  ui->rotationPhi->setValue(euler[2]);
  allowRotationUpdate = true;
}

void CrystalDisplay::slotLoadCellFromCrystal() {
  // Update space group symbol
  ui->Spacegroup->setText(crystal->getSpacegroup()->groupSymbol());

  // Get Cell from crystal
  QList<double> cell = crystal->getCell();

  //Prepare list of all inputs, that corresponts to the cell parameters
  QList<QDoubleSpinBox*> inputs;
  inputs << ui->latticeA << ui->latticeB << ui->latticeC << ui->latticeAlpha << ui->latticeBeta << ui->latticeGamma;

  // Bool to check, if the values change
  bool cellChanged = false;
  for (int i=0; i<6; i++) {
    // Block signals, to prevent multiple signals to be generated. Save the block state (should alwas be true)
    bool b = inputs[i]->blockSignals(true);
    // Save value for compare. Do not compare here, as the QSpinBox may round
    double v = inputs[i]->value();
    // set Value
    inputs[i]->setValue(cell[i]);
    // compare with former value. If not equal, cell is changed
    cellChanged = cellChanged || (v!=inputs[i]->value());
    // Restore blocking state
    inputs[i]->blockSignals(b);
  }
  // If Cell has changed, promote to crystal again
  if (cellChanged)
    slotCellChanged();
}

void CrystalDisplay::slotCellChanged() {
  QList<double> cell;
  cell << ui->latticeA->value();
  cell << ui->latticeB->value();
  cell << ui->latticeC->value();
  cell << ui->latticeAlpha->value();
  cell << ui->latticeBeta->value();
  cell << ui->latticeGamma->value();
  crystal->setCell(cell);
}

#include "ui/sadeasteregg.h"

void CrystalDisplay::slotSpacegroupChanged(QString s) {
  if (s=="JRS") {
    ui->Spacegroup->setText("Fd3m");
    ui->latticeA->setValue(5.43102064);
    SadEasterEgg see;
    see.exec();
  } else {
    setPaletteForStatus(ui->Spacegroup, crystal->getSpacegroup()->setGroupSymbol(s));
  }
}


void CrystalDisplay::slotRotationChanged() {
  if (allowRotationUpdate) {
    double omega = M_PI/180.0*ui->rotationOmega->value();
    double chi = M_PI/180.0*ui->rotationChi->value();
    double phi = M_PI/180.0*ui->rotationPhi->value();
    crystal->setEulerAngles(omega, chi, phi);
  }
}

void CrystalDisplay::slotSetSGConstrains() {
  QList<int> constrains = crystal->getSpacegroup()->getConstrains();
  QList<QDoubleSpinBox*> inputs;
  inputs << ui->latticeA << ui->latticeB << ui->latticeC << ui->latticeAlpha << ui->latticeBeta << ui->latticeGamma;
  for (int i=0; i<6; i++) {
    inputs[i]->setEnabled(constrains[i]==0);
  }
}

void CrystalDisplay::slotStartIndexing() {
  if (hklTool.isNull()) {
    hklTool = new HKLTool(crystal);
    connect(this, SIGNAL(destroyed()), hklTool, SLOT(deleteLater()));
    Clip::getInstance()->addMdiWindow(hklTool);
  } else {
    Clip::getInstance()->setActiveSubWindow(hklTool);
  }
}

void CrystalDisplay::on_actionDrag_hovered() {
  QDrag* drag = new QDrag(this);
  QMimeData* mime = new QMimeData;
  mime->setData("application/CrystalPointer", "");
  mime->setImageData(QVariant::fromValue(crystal));
  drag->setMimeData(mime);
  drag->exec(Qt::LinkAction);

}

void CrystalDisplay::dragEnterEvent(QDragEnterEvent *e) {
  if (e->mimeData()->hasFormat("application/CrystalPointer") && e->mimeData()->hasImage()) {
    QVariant v = e->mimeData()->imageData();
    if (v.canConvert<Crystal*>()) {
      Crystal* c = v.value<Crystal*>();
      if (c != crystal)
        e->acceptProposedAction();
    }
  }
}

void CrystalDisplay::dropEvent(QDropEvent *e) {
  if (e->mimeData()->hasImage()) {
    QVariant v = e->mimeData()->imageData();
    if (v.canConvert<Crystal*>()) {
      Crystal* c = v.value<Crystal*>();
      if (c!=crystal) {
        e->acceptProposedAction();
        *crystal = *c;
      }
    }
  }
}

const char XML_CrystalDisplay_Element[] = "CrystalDisplay";
const char XML_CrystalDisplay_Geometry[] = "Geometry";

void CrystalDisplay::loadFromXML(QDomElement base) {
  QDomElement element = base;
  if (element.tagName()!=XML_CrystalDisplay_Element)
    element = base.elementsByTagName(XML_CrystalDisplay_Element).at(0).toElement();
  if (element.isNull()) return;
  for (QDomElement e=element.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
    if (e.tagName()==XML_CrystalDisplay_Geometry) {
      if (QWidget* p = dynamic_cast<QWidget*>(parent()))
        p->setGeometry(TagToRect(e, p->geometry()));
    }
  }
  crystal->loadFromXML(element);
}

void CrystalDisplay::saveToXML(QDomElement base) {
  QDomElement cp = ensureElement(base, XML_CrystalDisplay_Element);
  if (QWidget* p = dynamic_cast<QWidget*>(parent())) {
    RectToTag(cp, XML_CrystalDisplay_Geometry, p->geometry());
  }
  crystal->saveToXML(cp);
}

void CrystalDisplay::loadDefault() {
  QDomDocument doc = readXMLFile(QString(":/DefaultCrystal.xml"));
  if (!doc.isNull()) {
    loadFromXML(doc.documentElement());
    slotLoadCellFromCrystal();
  }
}

void CrystalDisplay::on_actionLoad_triggered() {
  QSettings settings;
  QString filename = QFileDialog::getOpenFileName(this, "Load Cell Data", settings.value("LastDirectory").toString(),
                                                  "Contrast Curves (*.cell);;All Files (*)");

  QDomDocument doc = readXMLFile(filename);
  if (!doc.isNull()) {
    settings.setValue("LastDirectory", QFileInfo(filename).canonicalFilePath());
    crystal->loadFromXML(doc.documentElement());
  }
}

void CrystalDisplay::on_actionSave_triggered() {
  QSettings settings;
  QString filename = QFileDialog::getSaveFileName(this, "Choose File to save Cell", settings.value("LastDirectory").toString(), "Clip Cell files (*.cell);;All Files (*)");
  if (!filename.isEmpty()) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) return;

    settings.setValue("LastDirectory", QFileInfo(filename).canonicalFilePath());


    QDomDocument doc(Crystal::Settings_Group);
    QDomElement docElement = doc.appendChild(doc.createElement(Crystal::Settings_Group)).toElement();
    crystal->saveToXML(docElement);

    QTextStream ts(&file);
    doc.save(ts, 1);
    file.close();
  }
}
