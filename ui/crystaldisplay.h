#ifndef CRYSTALDISPLAY_H
#define CRYSTALDISPLAY_H

#include <QMainWindow>
#include <QPointer>
#include <QDomElement>

#include "ui/indexdisplay.h"

class Crystal;

namespace Ui {
  class CrystalDisplay;
}

class CrystalDisplay : public QMainWindow
{
  Q_OBJECT

public:
  explicit CrystalDisplay(QWidget *parent = 0);
  ~CrystalDisplay();

  void loadFromXML(QDomElement base);
  void saveToXML(QDomElement base);

  void resizeEvent(QResizeEvent *);

  QSize sizeHint() const;

  Crystal* getCrystal() { return crystal; }
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
  QPointer<IndexDisplay> indexDisplay;

private slots:
  void on_actionSave_triggered();
  void on_actionLoad_triggered();
  void on_actionDrag_hovered();
  void slotResizeOrientationMatrix();
};

#endif // CRYSTAL_H
