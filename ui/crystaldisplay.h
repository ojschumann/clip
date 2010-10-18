#ifndef CRYSTALDISPLAY_H
#define CRYSTALDISPLAY_H

#include <core/crystal.h>
#include <core/spacegroup.h>

#include <QWidget>

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

public slots:
    void slotUpdateOM();
    void slotCellChanged();
    void slotRotationChanged();
    void slotSpaceGroupChanged(QString);
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
