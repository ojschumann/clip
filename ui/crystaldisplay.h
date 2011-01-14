#ifndef CRYSTALDISPLAY_H
#define CRYSTALDISPLAY_H

#include <QMainWindow>
#include <QPointer>
#include <QDomElement>

#include "ui/hkltool.h"

class Crystal;

namespace Ui {
  class CrystalDisplay;
}

class CrystalDisplay : public QMainWindow
{
  Q_OBJECT

public:
  explicit CrystalDisplay(QWidget *parent = 0);
  virtual ~CrystalDisplay();

  void loadFromXML(QDomElement base);
  void saveToXML(QDomElement base);
  void loadDefault();

  void resizeEvent(QResizeEvent *);

  QSize sizeHint() const;

  Crystal* getCrystal() { return crystal; }

  virtual void dragEnterEvent(QDragEnterEvent *);
  virtual void dropEvent(QDropEvent *);

signals:
  void info(QString, int);
public slots:
  void slotUpdateOrientationMatrix();
  void slotCellChanged();
  void slotRotationChanged();
  void slotSpacegroupChanged(QString);
  void slotLoadCellFromCrystal();
  void slotLoadCrystalData();
  void slotSaveCrystalData();
  void slotStartIndexing();
  void slotSetSGConstrains();

private:
  Ui::CrystalDisplay *ui;
  Crystal* crystal;
  bool allowRotationUpdate;
  QPointer<HKLTool> hklTool;

private slots:
  void on_actionSave_triggered();
  void on_actionLoad_triggered();
  void on_actionDrag_hovered();
  void slotResizeOrientationMatrix();
};

#endif // CRYSTAL_H
