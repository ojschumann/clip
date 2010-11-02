#ifndef CRYSTALDISPLAY_H
#define CRYSTALDISPLAY_H

#include <QWidget>

class Crystal;

namespace Ui {
  class CrystalDisplay;
}

class CrystalDisplay : public QWidget
{
  Q_OBJECT

public:
  explicit CrystalDisplay(QWidget *parent = 0);
  ~CrystalDisplay();
  void resizeEvent(QResizeEvent *);
  void mousePressEvent(QMouseEvent *);

  QSize sizeHint() const;

  Crystal* getCrystal() {return crystal; }
signals:
  void info(QString, int);
public slots:
  void slotUpdateOM();
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
};

#endif // CRYSTAL_H
