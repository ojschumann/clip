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

#include <QMenu>
#include <QMenuBar>
#include <QTextCharFormat>
#include <QClipboard>
#include <QMimeData>
#include <QComboBox>
#include <QToolButton>
#include <QFontComboBox>
#include <QTextList>
#include <QColorDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPrintDialog>
#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QFileDialog>
#include <QFileInfo>

#include "core/projector.h"
#include "tools/combolineedit.h"

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

#include "core/crystal.h"

void PrintDialog::on_actionInsert_Cell_Table_triggered()
{
  if (projector && projector->getCrystal()) {

    QString tableCode = "";
    tableCode += "<table border=\"1\">";
    tableCode += "<tr><th>a</th><th>b</th><th>c</th></tr>";
    tableCode += "<tr><td>%1</td><td>%2</td><td>%3</td></tr>";
    tableCode += "<tr><th>alpha</th><th>beta</th><th>gamma</th></tr>";
    tableCode += "<tr><td>%4</td><td>%5</td><td>%6</td></tr>";
    tableCode += "</table>";

    QList<double> cell = projector->getCrystal()->getCell();
    foreach (double d, cell)
      tableCode = tableCode.arg(d);

    QTextCursor cursor(ui->textEdit->textCursor());
    cursor.beginEditBlock();
    cursor.insertHtml(tableCode);
    cursor.endEditBlock();
  }
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




QString PrintDialog::getFilename(QString &title, QString &suffix) {
  QString fileName = QFileDialog::getSaveFileName(this, title, printer->outputFileName(),
                                                QLatin1Char('*') + suffix);
  if (fileName.isEmpty()) return QString::null;
  if (QFileInfo(fileName).suffix().isEmpty())
    fileName.append(suffix);
  return fileName;
}

bool PrintDialog::doPrintout(QPrinter::OutputFormat format, const QString &title, const QString &suffix) {
  if (format==QPrinter::NativeFormat) {
    printer->setOutputFileName(QString::null);
  } else {
    QString fileName = getFilename(title, suffix);
    if (fileName.isNull()) return false;
    printer->setOutputFileName(fileName);
  }
  printer->setOutputFormat(format);
  preview->print();
  return true;
}

void PrintDialog::printToPrinter() {
  QPrintDialog* printDialog = new QPrintDialog(printer, this);
  if (printDialog->exec() == QDialog::Accepted) {
    doPrintout(QPrinter::NativeFormat);
  }
  delete printDialog;
}

void PrintDialog::quickPrintToPrinter() {
  doPrintout(QPrinter::NativeFormat);
}

void PrintDialog::printToPdf() {
  doPrintout(QPrinter::PdfFormat, "Export to PDF", ".pdf");
}

void PrintDialog::printToPS() {
  doPrintout(QPrinter::PostScriptFormat, "Export to Postscript", ".ps");
}

#include <QtSvg/QSvgGenerator>

void PrintDialog::printToSvg() {
  if (doPrintout("Export to Scalable Vector Graphics", ".svg")) {
    QSvgGenerator generator;
    generator.setFileName(printer->outputFileName());
    generator.setSize(printer->pageSize(QPrinter::Millimeter));

}

#include <QGraphicsView>
#include "image/laueimage.h"
#include "image/imagedatastore.h"
#include <QInputDialog>
#include <QScrollBar>

void PrintDialog::printToPng() {
  if (projector && !projector->getScene()->views().isEmpty() && doPrintout("Portable Network Graphics (PNG)", ".png")) {

    QGraphicsView *const firstView = projector->getScene()->views().at(0);


    int imgWidth;
    int imgHeight;

    if (projector->getLaueImage()) {
      QRectF visibleSceneRectF = QRectF(firstView->mapToScene(0, 0), firstView->mapToScene(firstView->viewport()->width(), firstView->viewport()->height())).normalized();
      QRectF sceneRect = projector->getScene()->sceneRect();
      QSizeF imageSize = projector->getLaueImage()->data()->getTransformedSizeData(ImageDataStore::PixelSize);
      qDebug() << visibleSceneRectF;
      qDebug() << sceneRect;
      qDebug() << firstView->mapToScene(0, 0) << firstView->mapToScene(firstView->viewport()->width(), firstView->viewport()->height());
      //visibleSceneRectF = sceneRect;
      imgWidth = imageSize.width()*visibleSceneRectF.width()/projector->getScene()->sceneRect().width() + 0.5;
      imgHeight = imageSize.height()*visibleSceneRectF.height()/projector->getScene()->sceneRect().height() + 0.5;
      qDebug() << imgWidth << imgHeight;
      qDebug() << firstView->mapToScene(0.5, 0.5) << firstView->mapToScene(-0.5, -0.5) << firstView->mapToScene(-1.0, - 1.0);
      if (firstView->horizontalScrollBar()) {
        qDebug() << "Scroll" << firstView->horizontalScrollBar()->minimum() << firstView->horizontalScrollBar()->value() << firstView->horizontalScrollBar()->maximum() << endl;
      }
    }
    else {
      imgWidth = 100;// QInputDialog::IntInputfirstView->viewport()->width();
      imgHeight = firstView->viewport()->height();
    }


    int textHeight = 0;
    qreal scale = 1.0;

    QTextDocument* d = ui->textEdit->document()->clone();
    if (!d->isEmpty()) {
      qreal desiredTextWidth = 2.0*d->documentMargin() + 80*QFontMetrics(QFont("Courier New", 8)).averageCharWidth();
      scale = 1.0*imgWidth/desiredTextWidth;
      d->setTextWidth(desiredTextWidth);
      textHeight = int(scale*d->size().height()) + 1;
    }

    QImage img(imgWidth, imgHeight+textHeight, QImage::Format_ARGB32);
    QPainter p(&img);

    if (!d->isEmpty()) {
      p.save();
      p.scale(scale, scale);
      d->drawContents(&p);
      p.restore();
    }

    emit paintRequested(&p, QRectF(0, textHeight, imgWidth, imgHeight));

    p.end();
    delete d;

    img.save(printer->outputFileName());


  }
}

void PrintDialog::printPreview(QPrinter *printer) {
  QSizeF pageSize = printer->pageRect(QPrinter::DevicePixel).size();

  QTextDocument* d = ui->textEdit->document()->clone();
  d->setPageSize(pageSize);

  int textHeight = 0;
  if (!d->isEmpty()) {
    textHeight = int(d->size().height())+1;
  }

  QPainter p(printer);
  if (!d->isEmpty())
    d->drawContents(&p);

  emit paintRequested(&p, QRectF(0, textHeight, pageSize.width(), pageSize.height()-textHeight));

  p.end();
  delete d;
}
