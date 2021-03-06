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

#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QMainWindow>
#include <QTextCharFormat>
#include <QPrinter>
#include <QTimer>

#include "config.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QFontComboBox)
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QMenu)
//QT_FORWARD_DECLARE_CLASS(QPrinter)
QT_FORWARD_DECLARE_CLASS(QPrintPreviewWidget)

QT_FORWARD_DECLARE_CLASS(Projector)

namespace Ui {
  class PrintDialog;
}

class PrintDialog : public QMainWindow
{
  Q_OBJECT

public:
  explicit PrintDialog(Projector*, QWidget* _parent = nullptr);
  ~PrintDialog();

private:
  Ui::PrintDialog *ui;
  void setupTextActions();
  void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
  void fontChanged(const QFont &f);
  void colorChanged(const QColor &c);
  void alignmentChanged(Qt::Alignment a);
  void verticalAlignmentChanged(QTextCharFormat::VerticalAlignment a);

  bool previewIsFitting();
  void previewSetFitting(bool);

signals:
  void paintRequested (QPainter *, const QRectF&);

private slots:
  void on_actionInsert_Image_info_triggered();
  void on_actionInsert_Projector_Info_triggered();
  void on_actionInsert_Cell_Table_triggered();

  void handleImageStatus();
  void imageMenuTrggered(QAction*);

  // Slots for formating the description
  void textBold();
  void textUnderline();
  void textItalic();
  void textFamily(const QString &f);
  void textSize(const QString &p);
  void textStyle(int styleIndex);
  void textColor();
  void textAlign(QAction *a);
  void textSubscript(bool);
  void textSuperscript(bool);
  void currentCharFormatChanged(const QTextCharFormat &format);
  void cursorPositionChanged();
  void clipboardDataChanged();

  // Slots for handling the Preview widget (stolen from QPrintPreviewDialog, hale the LGPL)
  void previewZoomFit(QAction *action);
  void previewZoomIn();
  void previewZoomOut();
  void previewZoomFactorChanged();
  void previewUpdateZoomFactor();
  void previewSetupPage();

  // Slots for Printing to various formats
  void printToDefaultPrinter();
  void printToPrinter();
  void printToPdf();
  void printToPS();
  void printToPng();
  void printToSvg();
  void printPreview(QPrinter *);

public:
  class PaintDeviceFactory {
  public:
    PaintDeviceFactory(const QString& description=QString::null, const QString& suffix=QString::null);
    virtual ~PaintDeviceFactory();
    virtual QPaintDevice* getDevice(int textHeight) = 0;
    virtual double desiredTextWidth() const = 0;
    virtual int deviceWidth() const = 0;
  protected:
    QString filename;
    bool fileChooserAborted;
  private:
    QString getFilename(const QString& title, const QString& suffix);
  };

private:
  void renderToPaintDevice(const PaintDeviceFactory&);
  QSize getImageSize(bool askForSize);

private:
  Projector* projector;

  QComboBox *comboStyle;
  QFontComboBox *comboFont;
  QComboBox *comboSize;
  QComboBox *zoomFactor;

  QActionGroup* fitGroup;

  QPrinter* printer;
  QPrintPreviewWidget* preview;

  QTimer updateTimer;
  bool updateScheduled;
private slots:
  void scheduleUpdate();
  void doUpdate();

};

#endif // PRINTDIALOG_H
