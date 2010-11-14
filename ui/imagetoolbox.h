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
private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_doCrop_clicked();
    void on_actionCrop_triggered();
};

#endif // IMAGETOOLBOX_H
