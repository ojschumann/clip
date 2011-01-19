#ifndef MOUSEINFODISPLAY_H
#define MOUSEINFODISPLAY_H

#include "tools/mousepositioninfo.h"

#include <QObject>
#include <QWidget>
#include <QPointer>

#include "tools/vec3D.h"

namespace Ui {
  class MouseInfoDisplay;
}

class MouseInfoDisplay : public QWidget
{
  Q_OBJECT

public:
  explicit MouseInfoDisplay(QWidget *parent = 0);
  virtual ~MouseInfoDisplay();
  virtual bool eventFilter(QObject *, QEvent *);
signals:
  void highlightMarker(Vec3D);
public slots:
  void showMouseInfo(MousePositionInfo);
  void receiveSpotHightlight(Vec3D);
protected:
  virtual void changeEvent(QEvent *);
private slots:
    void on_reflex_textEdited(QString );
    void cursorTableVisiblyToggled(bool b);
private:

  Ui::MouseInfoDisplay *ui;
  bool parentNeedSizeConstrain;

  QPointer<QObject> lastSender;
};

#endif // MOUSEINFODISPLAY_H
