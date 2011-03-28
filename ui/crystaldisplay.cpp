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


#include "tools/mat3D.h"
#include "core/crystal.h"
#include "core/spacegroup.h"
#include "ui/hkltool.h"
#include "ui/clip.h"
#include "tools/tools.h"
#include "tools/xmltools.h"

CrystalDisplay::CrystalDisplay(QWidget *parent) :
    QMainWindow(parent),
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
  double omega, phi, chi;
  crystal->calcEulerAngles(omega, chi, phi);
  allowRotationUpdate = false;
  ui->rotationOmega->setValue(180.0*M_1_PI*omega);
  ui->rotationChi->setValue(180.0*M_1_PI*chi);
  ui->rotationPhi->setValue(180.0*M_1_PI*phi);
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

void CrystalDisplay::slotSpacegroupChanged(QString s) {
  setPaletteForStatus(ui->Spacegroup, crystal->getSpacegroup()->setGroupSymbol(s));
  QPalette p = ui->Spacegroup->palette();
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


    QDomDocument doc("Crystal");
    QDomElement docElement = doc.appendChild(doc.createElement("Crystal")).toElement();
    crystal->saveToXML(docElement);

    QTextStream ts(&file);
    doc.save(ts, 1);
    file.close();
  }
}
