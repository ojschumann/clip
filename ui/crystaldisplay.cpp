#include "crystaldisplay.h"
#include "ui_crystaldisplay.h"
#include "core/mat3D.h"

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
    ui->tollBar->addAction(QIcon(":/fileopen.png"), "Open", this, SLOT(slotLoadCrystalData()));
    ui->tollBar->addAction(QIcon(":/filesave.png"), "Save", this, SLOT(slotSaveCrystalData()));
    ui->tollBar->addAction(QIcon(":/Index.png"), "Index", this, SLOT(slotStartIndexing()));

    // Load Orientation Matrix
    slotUpdateOM();

    // Load Cell
    slotLoadCellFromCrystal();
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
        QDrag drag(this);
        QMimeData mime;
        mime.setData("application/CrystalPointer", "");
        drag.setMimeData(&mime);
        drag.exec(Qt::LinkAction);
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
            ui->orientationMatrix->setItem(i,j, new QTableWidgetItem(QString::number(OM[i][j], 'f', 4)));
        }
    }
    // let Crystal calc Euler angles and write to UI
    double omega, phi, chi;
    crystal->calcEulerAngles(omega, chi, phi);
    allowRotationUpdate = false;
    ui->rotationOmega->setValue(M_PI/180.0*omega);
    ui->rotationChi->setValue(M_PI/180.0*chi);
    ui->rotationPhi->setValue(M_PI/180.0*phi);
    allowRotationUpdate = true;
}

void CrystalDisplay::slotLoadCellFromCrystal() {
    ui->SpaceGroup->setText(crystal->getSpacegroup()->groupSymbol());
    QList<double> cell = crystal->getCell();
    QList<QDoubleSpinBox*> inputs;
    inputs << ui->latticeA << ui->latticeB << ui->latticeC << ui->latticeAlpha << ui->latticeBeta << ui->latticeGamma;
    for (int i=0; i<6; i++)
        inputs[i]->setValue(cell[i]);
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
    crystal->getSpacegroup()->setGroupSymbol(s);
    slotSetSGConstrains();

/*
            if self.sg.system==self.sg.trigonal:
                self.crystal.setSpacegroupConstrains([0,0,0,0,0,0])
                if sgold.getCellConstrain()==[0,-1,-1,0,-4,-4] and self.sg.getCellConstrain()==[0,-1,0,90,90,120]:
                    self.R2T(self.crystal)
                elif sgold.getCellConstrain()==[0,-1,0,90,90,120] and self.sg.getCellConstrain()==[0,-1,-1,0,-4,-4]:
                    self.T2R(self.crystal)

            self.crystal.setSpacegroupConstrains(self.sg.getCellConstrain())

        constrain=self.sg.getCellConstrain()
        for i in range(6):
            self.inputs[i].setEnabled(constrain[i]==0)
        cell=self.sg.contrainCellToSymmetry(self.crystal.getCell())
        self.crystal.setCell(*cell)
*/

}


void CrystalDisplay::slotRotationChanged() {
    if (allowRotationUpdate) {
        double omega = 180.0*M_1_PI*ui->rotationOmega->value();
        double chi = 180.0*M_1_PI*ui->rotationChi->value();
        double phi = 180.0*M_1_PI*ui->rotationPhi->value();
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

    def R2T(self, Cr):
        a,b,c,al,be,ga=Cr.getCell()
        if a==b and b==c and al==be and be==ga:
            a=Cr.uvw2Real(1,-1,0)
            c=Cr.uvw2Real(1,1,1)
            Cr.setCell(a.norm(),a.norm(),c.norm(),90,90,120)
            ch=Cr.uvw2Real(0,0,1)
            Cr.addRotation(self.rotHT(a,c,Cr.uvw2Real(1,0,0),Cr.uvw2Real(0,0,1),-1))

    def T2R(self, Cr):
        a,b,c,al,be,ga=Cr.getCell()
        if a==b and al==90.0 and be==90.0 and ga==120.0:
            a=Cr.uvw2Real(2,1,1)/3
            b=Cr.uvw2Real(-1,1,1)/3
            ah=Cr.uvw2Real(1,0,0)
            ch=Cr.uvw2Real(0,0,1)
            l=a.norm()
            ang=math.degrees(math.acos(a*b/l/l))
            Cr.setCell(l,l,l,ang,ang,ang)
            Cr.addRotation(self.rotHT(Cr.uvw2Real(1,-1,0),Cr.uvw2Real(1,1,1),ah,ch,1))


    def rotHT(self,r1,r2,ah,ch,o):
        da=r1-ah
        dc=r2-ch

        n=da%dc
        n.normalize()

        c1=r2-n*(n*r2)
        c2=ch-n*(ch*n)
        c1.normalize()
        c2.normalize()
        R=Mat3D(n,o*math.acos(c1*c2))
        return R
*/
