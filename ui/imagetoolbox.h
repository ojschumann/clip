#ifndef IMAGETOOLBOX_H
#define IMAGETOOLBOX_H

#include <QMainWindow>

namespace Ui {
  class ImageToolbox;
}

class Projector;

class ImageToolbox : public QMainWindow
{
  Q_OBJECT

public:
  explicit ImageToolbox(Projector* p, QWidget *parent = 0);
  ~ImageToolbox();
   Ui::ImageToolbox *ui;
protected:
   Projector* projector;

};

#endif // IMAGETOOLBOX_H
