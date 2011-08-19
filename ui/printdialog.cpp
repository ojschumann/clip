/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

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

#include "printdialog.h"
#include "ui_printdialog.h"

#include <QTextCharFormat>
#include <QClipboard>
#include <QMimeData>
#include <QComboBox>
#include <QToolButton>
#include <QFontComboBox>
#include <QTextList>
#include <QColorDialog>
#include <QPainter>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPrintDialog>
#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QtSvg/QSvgGenerator>
#include <QGraphicsView>
#include <QInputDialog>
#include <QSettings>

#include "image/laueimage.h"
#include "image/imagedatastore.h"
#include "core/projector.h"
#include "tools/combolineedit.h"
#include "core/crystal.h"

class ZoomFactorValidator : public QDoubleValidator
{
public:
  ZoomFactorValidator(QObject* parent)
    : QDoubleValidator(parent) {}
  ZoomFactorValidator(qreal bottom, qreal top, int decimals, QObject *parent)
    : QDoubleValidator(bottom, top, decimals, parent) {}

  State validate(QString &input, int &pos) const
  {
    bool replacePercent = false;
    if (input.endsWith(QLatin1Char('%'))) {
      input = input.left(input.length() - 1);
      replacePercent = true;
    }
    State state = QDoubleValidator::validate(input, pos);
    if (replacePercent)
      input += QLatin1Char('%');
    const int num_size = 4;
    if (state == Intermediate) {
      int i = input.indexOf(QLocale::system().decimalPoint());
      if ((i == -1 && input.size() > num_size)
        || (i != -1 && i > num_size))
        return Invalid;
    }
    return state;
  }
};



PrintDialog::PrintDialog(Projector* p, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PrintDialog),
    projector(p)
{
  ui->setupUi(this);
  connect(p, SIGNAL(destroyed()), this, SLOT(deleteLater()));

  printer = new QPrinter(QPrinterInfo::defaultPrinter());
  printer->setPageMargins(10, 10, 10, 10, QPrinter::Millimeter);
  preview = new QPrintPreviewWidget(printer, ui->previewFrame);

  connect(ui->actionLandscape, SIGNAL(triggered()), preview, SLOT(setLandscapeOrientation()));
  connect(ui->actionPortrait, SIGNAL(triggered()), preview, SLOT(setPortraitOrientation()));
  connect(ui->textEdit, SIGNAL(textChanged()), preview, SLOT(updatePreview()));
  connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printPreview(QPrinter*)));
  connect(preview, SIGNAL(previewChanged()), this, SLOT(previewUpdateZoomFactor()));

  QVBoxLayout* l = new QVBoxLayout(ui->previewFrame);
  l->addWidget(preview);
  l->setMargin(0);



  setToolButtonStyle(Qt::ToolButtonFollowStyle);

  ui->actionUndo->setShortcut(QKeySequence::Undo);
  ui->actionRedo->setShortcut(QKeySequence::Redo);
  ui->actionTextBold->setShortcut(QKeySequence::Bold);
  ui->actionTextItalic->setShortcut(QKeySequence::Italic);
  ui->actionTextUnderline->setShortcut(QKeySequence::Underline);

  setupTextActions();

  ui->textEdit->setFocus();
  QFile f(":/report.css");
  f.open(QIODevice::ReadOnly);
  ui->textEdit->document()->setDefaultStyleSheet(QString(f.readAll()));
  f.close();

  fontChanged(ui->textEdit->font());
  colorChanged(ui->textEdit->textColor());
  alignmentChanged(ui->textEdit->alignment());
  verticalAlignmentChanged(ui->textEdit->currentCharFormat().verticalAlignment());
  clipboardDataChanged();

  connect(ui->textEdit->document(), SIGNAL(undoAvailable(bool)),
          ui->actionUndo, SLOT(setEnabled(bool)));
  connect(ui->textEdit->document(), SIGNAL(redoAvailable(bool)),
          ui->actionRedo, SLOT(setEnabled(bool)));

  ui->actionUndo->setEnabled(ui->textEdit->document()->isUndoAvailable());
  ui->actionRedo->setEnabled(ui->textEdit->document()->isRedoAvailable());

  ui->actionCut->setEnabled(false);
  ui->actionCopy->setEnabled(false);

#ifndef QT_NO_CLIPBOARD
  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
#endif



  zoomFactor = new QComboBox(this);
  zoomFactor->setEditable(true);
  zoomFactor->setMinimumContentsLength(7);
  zoomFactor->setInsertPolicy(QComboBox::NoInsert);
  ComboLineEdit *zoomEditor = new ComboLineEdit;
  zoomEditor->setValidator(new ZoomFactorValidator(1, 1000, 1, zoomEditor));
  zoomFactor->setLineEdit(zoomEditor);
  static const short factorsX2[] = { 25, 50, 100, 200, 250, 300, 400, 800, 1600 };
  for (int i = 0; i < int(sizeof(factorsX2) / sizeof(factorsX2[0])); ++i)
    zoomFactor->addItem(PrintDialog::tr("%1%").arg(0.5*factorsX2[i]));
  connect(zoomFactor->lineEdit(), SIGNAL(editingFinished()), this, SLOT(previewZoomFactorChanged()));
  connect(zoomFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(previewZoomFactorChanged()));

  ui->previewToolbar->insertWidget(ui->actionZoomPreviewIn, zoomFactor);

  // Cannot use the actions' triggered signal here, since it doesn't autorepeat
  QToolButton *zoomInButton = static_cast<QToolButton *>(ui->previewToolbar->widgetForAction(ui->actionZoomPreviewIn));
  QToolButton *zoomOutButton = static_cast<QToolButton *>(ui->previewToolbar->widgetForAction(ui->actionZoomPreviewOut));
  zoomInButton->setAutoRepeat(true);
  zoomInButton->setAutoRepeatInterval(200);
  zoomInButton->setAutoRepeatDelay(200);
  zoomOutButton->setAutoRepeat(true);
  zoomOutButton->setAutoRepeatInterval(200);
  zoomOutButton->setAutoRepeatDelay(200);
  connect(zoomInButton, SIGNAL(clicked()), this, SLOT(previewZoomIn()));
  connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(previewZoomOut()));

  fitGroup = new QActionGroup(this);
  fitGroup->addAction(ui->actionFitPage);
  fitGroup->addAction(ui->actionFitWidth);
  connect(fitGroup, SIGNAL(triggered(QAction*)), this, SLOT(previewZoomFit(QAction*)));

  QActionGroup* ag = new QActionGroup(this);
  ag->setExclusive(true);
  ag->addAction(ui->actionPortrait);
  ag->addAction(ui->actionLandscape);

  ui->actionFitPage->setChecked(true);
  if (preview->orientation() == QPrinter::Portrait)
      ui->actionPortrait->setChecked(true);
  else
      ui->actionLandscape->setChecked(true);
}

PrintDialog::~PrintDialog()
{
  delete ui;
  delete printer;
}

const QString rsrcPath = ":/icons/icons/";


void PrintDialog::setupTextActions()
{
  QActionGroup *grp = new QActionGroup(this);
  connect(grp, SIGNAL(triggered(QAction*)), this, SLOT(textAlign(QAction*)));
  grp->addAction(ui->actionAlignLeft);
  grp->addAction(ui->actionAlignCenter);
  grp->addAction(ui->actionAlignRight);
  grp->addAction(ui->actionAlignJustify);


  comboStyle = new QComboBox(ui->formatToolbar);
  ui->formatToolbar->addWidget(comboStyle);
  comboStyle->addItem("Standard");
  comboStyle->addItem("Bullet List (Disc)");
  comboStyle->addItem("Bullet List (Circle)");
  comboStyle->addItem("Bullet List (Square)");
  comboStyle->addItem("Ordered List (Decimal)");
  comboStyle->addItem("Ordered List (Alpha lower)");
  comboStyle->addItem("Ordered List (Alpha upper)");
  comboStyle->addItem("Ordered List (Roman lower)");
  comboStyle->addItem("Ordered List (Roman upper)");
  connect(comboStyle, SIGNAL(activated(int)),
          this, SLOT(textStyle(int)));

  comboFont = new QFontComboBox(ui->formatToolbar);
  ui->formatToolbar->addWidget(comboFont);
  connect(comboFont, SIGNAL(activated(QString)),
          this, SLOT(textFamily(QString)));

  comboSize = new QComboBox(ui->formatToolbar);
  comboSize->setObjectName("comboSize");
  ui->formatToolbar->addWidget(comboSize);
  comboSize->setEditable(true);

  QFontDatabase db;
  foreach(int size, db.standardSizes())
    comboSize->addItem(QString::number(size));

  connect(comboSize, SIGNAL(activated(QString)),
          this, SLOT(textSize(QString)));
  comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font().pointSize())));
}

void PrintDialog::textBold()
{
  QTextCharFormat fmt;
  fmt.setFontWeight(ui->actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
  mergeFormatOnWordOrSelection(fmt);
}

void PrintDialog::textUnderline()
{
  QTextCharFormat fmt;
  fmt.setFontUnderline(ui->actionTextUnderline->isChecked());
  mergeFormatOnWordOrSelection(fmt);
}

void PrintDialog::textItalic()
{
  QTextCharFormat fmt;
  fmt.setFontItalic(ui->actionTextItalic->isChecked());
  mergeFormatOnWordOrSelection(fmt);
}

void PrintDialog::textSubscript(bool b) {
  QTextCharFormat fmt;
  if (b) {
    ui->actionSuperscript->setChecked(false);
    fmt.setVerticalAlignment(QTextCharFormat::AlignSubScript);
  } else {
    fmt.setVerticalAlignment(QTextCharFormat::AlignNormal);
  }
  ui->textEdit->mergeCurrentCharFormat(fmt);
}

void PrintDialog::textSuperscript(bool b) {
  QTextCharFormat fmt;
  if (b) {
    ui->actionSubscript->setChecked(false);
    fmt.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
  } else {
    fmt.setVerticalAlignment(QTextCharFormat::AlignNormal);
  }
  ui->textEdit->mergeCurrentCharFormat(fmt);
}

void PrintDialog::textFamily(const QString &f)
{
  QTextCharFormat fmt;
  fmt.setFontFamily(f);
  mergeFormatOnWordOrSelection(fmt);
}

void PrintDialog::textSize(const QString &p)
{
  qreal pointSize = p.toFloat();
  if (p.toFloat() > 0) {
    QTextCharFormat fmt;
    fmt.setFontPointSize(pointSize);
    mergeFormatOnWordOrSelection(fmt);
  }
}

void PrintDialog::textStyle(int styleIndex)
{
  QTextCursor cursor = ui->textEdit->textCursor();

  if (styleIndex != 0) {
    QTextListFormat::Style style = QTextListFormat::ListDisc;

    switch (styleIndex) {
    default:
    case 1:
      style = QTextListFormat::ListDisc;
      break;
    case 2:
      style = QTextListFormat::ListCircle;
      break;
    case 3:
      style = QTextListFormat::ListSquare;
      break;
    case 4:
      style = QTextListFormat::ListDecimal;
      break;
    case 5:
      style = QTextListFormat::ListLowerAlpha;
      break;
    case 6:
      style = QTextListFormat::ListUpperAlpha;
      break;
    case 7:
      style = QTextListFormat::ListLowerRoman;
      break;
    case 8:
      style = QTextListFormat::ListUpperRoman;
      break;
    }

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextListFormat listFmt;

    if (cursor.currentList()) {
      listFmt = cursor.currentList()->format();
    } else {
      listFmt.setIndent(blockFmt.indent() + 1);
      blockFmt.setIndent(0);
      cursor.setBlockFormat(blockFmt);
    }

    listFmt.setStyle(style);

    cursor.createList(listFmt);

    cursor.endEditBlock();
  } else {
    // ####
    QTextBlockFormat bfmt;
    bfmt.setObjectIndex(-1);
    cursor.mergeBlockFormat(bfmt);
  }
}

void PrintDialog::textColor()
{
  QColor col = QColorDialog::getColor(ui->textEdit->textColor(), this);
  if (!col.isValid())
    return;
  QTextCharFormat fmt;
  fmt.setForeground(col);
  mergeFormatOnWordOrSelection(fmt);
  colorChanged(col);
}

void PrintDialog::textAlign(QAction *a)
{
  if (a == ui->actionAlignLeft)
    ui->textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
  else if (a == ui->actionAlignCenter)
    ui->textEdit->setAlignment(Qt::AlignHCenter);
  else if (a == ui->actionAlignRight)
    ui->textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
  else if (a == ui->actionAlignJustify)
    ui->textEdit->setAlignment(Qt::AlignJustify);
}


void PrintDialog::currentCharFormatChanged(const QTextCharFormat &format)
{
  fontChanged(format.font());
  colorChanged(format.foreground().color());
}

void PrintDialog::cursorPositionChanged()
{
  alignmentChanged(ui->textEdit->alignment());
  verticalAlignmentChanged(ui->textEdit->currentCharFormat().verticalAlignment());
}

void PrintDialog::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
  if (const QMimeData *md = QApplication::clipboard()->mimeData())
    ui->actionPaste->setEnabled(md->hasText());
#endif
}

void PrintDialog::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
  QTextCursor cursor = ui->textEdit->textCursor();
  if (!cursor.hasSelection())
    cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
  ui->textEdit->mergeCurrentCharFormat(format);
}

void PrintDialog::fontChanged(const QFont &f)
{
  comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
  comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
  ui->actionTextBold->setChecked(f.bold());
  ui->actionTextItalic->setChecked(f.italic());
  ui->actionTextUnderline->setChecked(f.underline());
}

void PrintDialog::colorChanged(const QColor &c)
{
  QPixmap pix(16, 16);
  pix.fill(c);
  ui->actionTextColor->setIcon(pix);
}

void PrintDialog::alignmentChanged(Qt::Alignment a)
{
  if (a & Qt::AlignLeft) {
    ui->actionAlignLeft->setChecked(true);
  } else if (a & Qt::AlignHCenter) {
    ui->actionAlignCenter->setChecked(true);
  } else if (a & Qt::AlignRight) {
    ui->actionAlignRight->setChecked(true);
  } else if (a & Qt::AlignJustify) {
    ui->actionAlignJustify->setChecked(true);
  }
}

void PrintDialog::verticalAlignmentChanged(QTextCharFormat::VerticalAlignment a)
{
  ui->actionSubscript->setChecked(a == QTextCharFormat::AlignSubScript);
  ui->actionSuperscript->setChecked(a == QTextCharFormat::AlignSuperScript);
}



void PrintDialog::previewZoomFactorChanged() {
  QString text = zoomFactor->lineEdit()->text();
  bool ok;
  qreal factor = text.remove(QLatin1Char('%')).toFloat(&ok);
  factor = qMax(qreal(1.0), qMin(qreal(1000.0), factor));
  if (ok) {
    preview->setZoomFactor(factor/100.0);
    zoomFactor->setEditText(QString::fromLatin1("%1%").arg(factor));
    previewSetFitting(false);
  }

}


void PrintDialog::previewZoomIn() {
  previewSetFitting(false);
  preview->zoomIn();
  previewUpdateZoomFactor();
}

void PrintDialog::previewZoomOut() {
  previewSetFitting(false);
  preview->zoomOut();
  previewUpdateZoomFactor();
}

void PrintDialog::previewZoomFit(QAction* action) {
  previewSetFitting(true);
  if (action == ui->actionFitPage)
    preview->fitInView();
  else
    preview->fitToWidth();
}

bool PrintDialog::previewIsFitting() {
  return (fitGroup->isExclusive() && (ui->actionFitPage->isChecked() || ui->actionFitWidth->isChecked()));
}


void PrintDialog::previewSetFitting(bool on) {
  if (previewIsFitting() == on)
    return;
  fitGroup->setExclusive(on);
  if (on) {
    QAction* action = ui->actionFitWidth->isChecked() ? ui->actionFitWidth : ui->actionFitPage;
    action->setChecked(true);
    if (fitGroup->checkedAction() != action) {
      // work around exclusitivity problem
      fitGroup->removeAction(action);
      fitGroup->addAction(action);
    }
  } else {
    ui->actionFitWidth->setChecked(false);
    ui->actionFitPage->setChecked(false);
  }
}

void PrintDialog::previewUpdateZoomFactor() {
  zoomFactor->lineEdit()->setText(QString().sprintf("%.1f%%", preview->zoomFactor()*100));
}

void PrintDialog::previewSetupPage() {
  QPageSetupDialog pageSetup(printer, this);
  if (pageSetup.exec() == QDialog::Accepted) {
    // update possible orientation changes
    if (preview->orientation() == QPrinter::Portrait) {
      ui->actionPortrait->setChecked(true);
      preview->setPortraitOrientation();
    }else {
      ui->actionLandscape->setChecked(true);
      preview->setLandscapeOrientation();
    }
  }
}

int numberOfDecimalPlaces(double d) {
  int m=0;
  d -= trunc(d);
  while (fabs(d)>1e-10) {
    d *= 10.0;
    d -= trunc(d);
    m++;
  }

  return m;
}

#include "tools/zipiterator.h"

void doReplace(QString& code, const QStringList& fields, const QList<double>& values, int maxDigits, bool handleAngles=false) {

  int digits = 0;
  foreach (double d, values) digits = qMin(maxDigits, qMax(digits, numberOfDecimalPlaces(d)));

  QPair<QString,double> p;
  foreach(p, Zip(fields, values)) code.replace(p.first, QString::number(p.second, 'f', (handleAngles && ((fabs(p.second-90.0)<1e-7)||(fabs(p.second-120.0)<1e-7))?0:digits) ));

}

void PrintDialog::on_actionInsert_Cell_Table_triggered()
{
  if (projector && projector->getCrystal()) {

    Crystal* c = projector->getCrystal();

    QFile f(":/report_crystal.html");
    f.open(QIODevice::ReadOnly);
    QString tableCode = QString(f.readAll());
    f.close();

    tableCode.replace("<SPACEGROUP/>", c->getSpacegroup()->groupSymbol());
    QList<double> cell=c->getCell();

    doReplace(tableCode, QStringList() << "<CELL_A/>" << "<CELL_B/>" << "<CELL_C/>", cell.mid(0, 3), 5);
    doReplace(tableCode, QStringList() << "<CELL_ALPHA/>" << "<CELL_BETA/>" << "<CELL_GAMMA/>", cell.mid(3, 3), 5, true);
    doReplace(tableCode, QStringList() << "<ORIENTATION_OMEGA/>" << "<ORIENTATION_CHI/>" << "<ORIENTATION_PHI/>", c->calcEulerAngles(true), 3);

    QTextCursor cursor(ui->textEdit->textCursor());
    cursor.beginEditBlock();
    cursor.insertHtml(tableCode);
    cursor.endEditBlock();
  }
}


QSize PrintDialog::getImageSize(bool askForSize) {
  if (projector && !projector->getScene()->views().isEmpty() && projector->getLaueImage()) {
    QGraphicsView *const firstView = projector->getScene()->views().at(0);

    QRectF visibleSceneRectF = QRectF(firstView->mapToScene(0, 0), firstView->mapToScene(firstView->viewport()->width(), firstView->viewport()->height())).normalized();
    QSizeF imageSize = projector->getLaueImage()->data()->getTransformedSizeData(ImageDataStore::PixelSize);
    imageSize.rwidth() *= visibleSceneRectF.width()/projector->getScene()->sceneRect().width();
    imageSize.rheight() *= visibleSceneRectF.height()/projector->getScene()->sceneRect().height();
    imageSize += QSizeF(0.5, 0.5);
    return imageSize.toSize();
  }
  return QSize(100, 100);
}

PrintDialog::PaintDeviceFactory::PaintDeviceFactory(const QString &description, const QString &suffix): filename(QString::null), fileChooserAborted(false) {
  if (!description.isNull()) {
    filename = getFilename(description, suffix);
    fileChooserAborted = filename.isNull();
  }
}

PrintDialog::PaintDeviceFactory::~PaintDeviceFactory() {
  QFileInfo fInfo(filename);
  if (!filename.isEmpty() && fInfo.exists())
    QSettings().setValue("LastDirectory", fInfo.canonicalFilePath());
}

QString PrintDialog::PaintDeviceFactory::getFilename(const QString &title, const QString &suffix) {

  QString fileName = QFileDialog::getSaveFileName(NULL, title, QSettings().value("LastDirectory").toString(), QLatin1Char('*') + suffix);
  if (fileName.isEmpty()) return QString::null;
  ;

  if (QFileInfo(fileName).suffix().isEmpty())
    fileName.append(suffix);

  return fileName;

}

class PrinterDevice: public PrintDialog::PaintDeviceFactory {
public:
  PrinterDevice(QPrinter* _p, QPrinter::OutputFormat f=QPrinter::NativeFormat, const QString& description=QString::null, const QString& sufffix=QString::null):
      PaintDeviceFactory(description, sufffix),
      printer(_p),
      format(f) {}
  ~PrinterDevice() { printer->setOutputFormat(QPrinter::NativeFormat); }
  virtual QPaintDevice* getDevice(int) {
    if (fileChooserAborted) return NULL;
    printer->setOutputFormat(format);
    printer->setOutputFileName(filename);
    return printer;
  }
  virtual double desiredTextWidth() const { return printer->pageRect(QPrinter::DevicePixel).width(); }
  virtual int deviceWidth() const { return printer->pageRect(QPrinter::DevicePixel).width(); }
private:
  QPrinter* printer;
  QPrinter::OutputFormat format;
};

class ImageDevice: public PrintDialog::PaintDeviceFactory {
public:
  ImageDevice(Projector* p, const QString& description, const QString& suffix);
  ~ImageDevice() { if (device) { delete device; device=0; } }
  virtual double desiredTextWidth() const { return 80*QFontMetrics(QFont("Courier New", 8)).averageCharWidth(); }
  virtual int deviceWidth() const { return imageSize.width(); }
  QPaintDevice* getDevice(int textHeight) {
    if (filename.isNull()) return NULL;
    if (!device) setupDevice(textHeight);
    return device;
  }
protected:
  virtual void setupDevice(int textHeight) = 0;
  //Projector* projector;
  QSize imageSize;
  QPaintDevice* device;
};

ImageDevice::ImageDevice(Projector *projector, const QString& description, const QString& suffix): PaintDeviceFactory(description, suffix), device(0) {
  if (projector && !projector->getScene()->views().isEmpty()) {
    QGraphicsView *const firstView = projector->getScene()->views().at(0);
    QRectF visibleSceneRectF = QRectF(firstView->mapToScene(0, 0), firstView->mapToScene(firstView->viewport()->width(), firstView->viewport()->height())).normalized();
    if (projector->getLaueImage()) {
      QSizeF displayedImageSize = projector->getLaueImage()->data()->getTransformedSizeData(ImageDataStore::PixelSize);
      displayedImageSize.rwidth() *= visibleSceneRectF.width()/projector->getScene()->sceneRect().width();
      displayedImageSize.rheight() *= visibleSceneRectF.height()/projector->getScene()->sceneRect().height();
      imageSize = (displayedImageSize + QSizeF(0.5, 0.5)).toSize();
    } else {
      imageSize = (visibleSceneRectF.size() * 800/visibleSceneRectF.width()).toSize();
    }
  } else {
    imageSize = QSize(600, 800);
  }
}

class PngDevice: public ImageDevice {
public:
  PngDevice(Projector* p): ImageDevice(p, "Export as Portable Network Graphics", ".png") { }
  ~PngDevice() { if (device) dynamic_cast<QImage*>(device)->save(filename); }
  virtual void setupDevice(int textHeight) {
    device = new QImage(imageSize.width(), imageSize.height()+textHeight, QImage::Format_ARGB32);
    static_cast<QImage*>(device)->fill(qRgb(0xFF, 0xFF, 0xFF));
  }
};

class SvgDevice: public ImageDevice {
public:
  SvgDevice(Projector* p): ImageDevice(p, "Export as Scalable Vector Graphics", ".svg") { }
  virtual void setupDevice(int textHeight) {
    device = new QSvgGenerator();
    static_cast<QSvgGenerator*>(device)->setFileName(filename);
    static_cast<QSvgGenerator*>(device)->setSize(QSize(imageSize.width(), imageSize.height()+textHeight));
    static_cast<QSvgGenerator*>(device)->setViewBox(QRect(0, 0, imageSize.width(), imageSize.height()+textHeight));
  }
};

void PrintDialog::printToPrinter() {
  QPrintDialog* printDialog = new QPrintDialog(printer, this);
  if (printDialog->exec() == QDialog::Accepted) {
    renderToPaintDevice(PrinterDevice(printer));
  }
  delete printDialog;
}

void PrintDialog::printToDefaultPrinter() {
  renderToPaintDevice(PrinterDevice(printer));
}

void PrintDialog::printPreview(QPrinter *p) {
  renderToPaintDevice(PrinterDevice(p));
}

void PrintDialog::printToPdf() {
  renderToPaintDevice(PrinterDevice(printer, QPrinter::PdfFormat, "Export to PDF", ".pdf"));
}

void PrintDialog::printToPS() {
  renderToPaintDevice(PrinterDevice(printer, QPrinter::PostScriptFormat, "Export to Postscript", ".ps"));
}

void PrintDialog::printToPng() {
  renderToPaintDevice(PngDevice(projector));
}

void PrintDialog::printToSvg() {
  renderToPaintDevice(SvgDevice(projector));
}

void PrintDialog::renderToPaintDevice(const PaintDeviceFactory& _factory) {
  PaintDeviceFactory& factory = const_cast<PaintDeviceFactory&>(_factory);
  int textHeight = 0;
  qreal scale = 1.0;

  QScopedPointer<QTextDocument> d(ui->textEdit->document()->clone());
  if (!d->isEmpty()) {
    scale = 1.0*factory.deviceWidth()/factory.desiredTextWidth();
    d->setTextWidth(factory.desiredTextWidth());
    textHeight = int(scale*d->size().height()) + 1;
  }

  QPaintDevice* device = factory.getDevice(textHeight);
  if (!device) return;
  QPainter p(device);

  if (!d->isEmpty()) {
    p.save();
    p.scale(scale, scale);
    p.setBackground(Qt::white);
    d->drawContents(&p);
    p.restore();
  }

  emit paintRequested(&p, QRectF(0, textHeight, device->width(), device->height()+textHeight));
}


