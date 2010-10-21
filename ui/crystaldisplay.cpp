#include "crystaldisplay.h"
#include "ui_crystaldisplay.h"
#include <tools/mat3D.h>

#include <QTableWidgetItem>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

CrystalDisplay::CrystalDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CrystalDisplay), crystal(new Crystal(this)), allowRotationUpdate(true)
{
  ui->setupUi(this);

  // Connect Signals of the crystal
  connect(crystal, SIGNAL(orientationChanged()), this, SLOT(slotUpdateOM()));
  connect(crystal, SIGNAL(cellChanged()), this, SLOT(slotUpdateOM()));
  connect(crystal, SIGNAL(cellChanged()), this, SLOT(slotLoadCellFromCrystal()));

  connect(crystal->getSpacegroup(), SIGNAL(constrainsChanged()), this, SLOT(slotSetSGConstrains()));

  // Add Toolbar buttons
  ui->tollBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open Crystal Data", this, SLOT(slotLoadCrystalData()));
  ui->tollBar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Save Crystal Data", this, SLOT(slotSaveCrystalData()));
  ui->tollBar->addAction(QIcon(":/Index.png"), "Index", this, SLOT(slotStartIndexing()));

  // Init Orientation Matrix Elements
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      ui->orientationMatrix->setItem(i,j, new QTableWidgetItem(""));

  // Load Orientation Matrix
  slotUpdateOM();

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


void CrystalDisplay::resizeEvent(QResizeEvent *) {
  // Adjust Size of the Orientation Matrix Cells
  int w = ui->orientationMatrix->width()/3;
  int h = ui->orientationMatrix->height()/3;
  for (int i=0; i<3; i++) {
    ui->orientationMatrix->setColumnWidth(i, w);
    ui->orientationMatrix->setRowHeight(i, h);
  }
}

void CrystalDisplay::mousePressEvent(QMouseEvent* e) {
  if (e->button()==Qt::LeftButton and ui->dragStart->geometry().contains(e->pos())) {
    QDrag* drag = new QDrag(this);
    QMimeData* mime = new QMimeData;
    mime->setData("application/CrystalPointer", "");
    mime->setImageData(QVariant::fromValue(crystal));
    drag->setMimeData(mime);
    drag->exec(Qt::LinkAction);
  }
}

QSize CrystalDisplay::sizeHint() const {
  // sizeHint results in a compact window
  return minimumSizeHint();
}

void CrystalDisplay::slotUpdateOM() {
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
  ui->SpaceGroup->setText(crystal->getSpacegroup()->groupSymbol());

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

void CrystalDisplay::slotSpaceGroupChanged(QString s) {
  QPalette p = ui->SpaceGroup->palette();
  if (crystal->getSpacegroup()->setGroupSymbol(s)) {
    p.setColor(QPalette::Base, Qt::white);
  } else {
    p.setColor(QPalette::Base, QColor(255, 200, 200));
  }
  ui->SpaceGroup->setPalette(p);
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

void CrystalDisplay::slotLoadCrystalData() {

}

void CrystalDisplay::slotSaveCrystalData() {

}

void CrystalDisplay::slotStartIndexing() {
  /*    w=Indexing(self.crystal)
    mdi=self.parent().mdiArea()
    mdi.addSubWindow(w)
    w.show() */
}



/*
    TODO: Port

    def crystaldata2xml(self, w):
        w.writeStartElement('Crystal')

        w.writeEmptyElement('Spacegroup')
        w.writeAttribute('symbol', str(self.crystal.getSpacegroupSymbol()))

        w.writeEmptyElement('Cell')
        for val, name in zip(self.crystal.getCell(), ('a', 'b', 'c', 'alpha', 'beta', 'gamma')):
            w.writeAttribute(name, str(val))

        w.writeEmptyElement('Orientation')
        for val, name in zip(self.crystal.calcEulerAngles()[:3], ('omega', 'chi','phi')):
            w.writeAttribute(name, str(math.degrees(val)))

        w.writeEndElement()

    def loadFromXML(self, r):
        if r.name()!="Crystal" or not r.isStartElement():
            return
        while not r.atEnd() and not (r.isEndElement() and r.name()=="Crystal"):
            if r.readNext()==QtCore.QXmlStreamReader.StartElement:
                if r.name()=="Spacegroup":
                    s=r.attributes().value('symbol')
                    if not s.isNull():
                        self.crystal.setSpacegroupSymbol(str(s.toString()))
                elif r.name()=="Cell":
                    cell=[]
                    for name in ('a', 'b', 'c', 'alpha', 'beta', 'gamma'):
                        s=r.attributes().value(name)
                        v, b=s.toString().toDouble()
                        if not s.isNull() and b:
                            cell.append(v)
                    if len(cell)==6:
                        self.crystal.setCell(*cell)
                elif r.name()=="Orientation":
                    angles=[]
                    for name in ('omega', 'chi', 'phi'):
                        s=r.attributes().value(name)
                        v, b=s.toString().toDouble()
                        if not s.isNull() and b:
                            angles.append(math.radians(v))
                    if len(angles)==3:
                        self.crystal.setEulerAngles(*angles)

    def slotSaveCrystalData(self):
        xmlString=QtCore.QString()
        w=QtCore.QXmlStreamWriter(xmlString)
        w.setAutoFormatting(True)
        w.setAutoFormattingIndent(2)

        self.crystaldata2xml(w)
        w.writeEndDocument()
        print xmlString

        fileName = QtGui.QFileDialog.getSaveFileName(self, 'Choose File to save Cell', '', 'Clip Cell files (*.cell);;All Files (*)')
        if fileName!="":
            f=open(str(fileName),  'w')
            f.write(str(xmlString))
            f.close()


    def slotOpenCrystalData(self):
        fileName = str(QtGui.QFileDialog.getOpenFileName(self, 'Choose Cell to load from File', '', 'Clip Cell files (*.cell);;All Files (*)'))
        try:
            f=open(str(fileName))
            s=''.join(f.readlines())
        except:
            return
        else:
            r=QtCore.QXmlStreamReader(s)
            while not r.atEnd():
                r.readNext()
                if r.name()=='Crystal' and r.isStartElement():
                    self.loadFromXML(r)


*/
