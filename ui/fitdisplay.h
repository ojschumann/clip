#ifndef FITDISPLAY_H
#define FITDISPLAY_H

#include <QMainWindow>

#include "tools/vec3D.h"
#include "tools/mat3D.h"

class Crystal;
class FitParameter;
class AbstractMarkerItem;
class Vertex;


namespace Ui {
  class FitDisplay;
}

class FitDisplay : public QMainWindow
{
  Q_OBJECT

public:
  explicit FitDisplay(Crystal* c, QWidget *parent = 0);
  ~FitDisplay();

private slots:
  void on_doFit_clicked();
  void updateTransferMatrix();

private:
  struct FitMarker {
    AbstractMarkerItem* marker;
    Vec3D index;
    double index_sq;
  };
  class Vertex {
  public:
    Vertex();
    Vertex(int N);
    Vertex(const Vertex& o);
    double score;
    QVector<double> coordinates;

    bool operator<(const Vertex& o) const;
    Vertex& operator=(const Vertex& o);
    Vertex& operator+=(const Vertex& o);
    Vertex& operator-=(const Vertex& o);
    Vertex& operator*=(double scale);
    Vertex operator+(const Vertex& o);
    Vertex operator-(const Vertex& o);
    Vertex operator*(double scale) const;

    double at(int n) const;
    int size() const;
  };

  double score();
  double score(Vertex& v);

  Ui::FitDisplay *ui;
  Crystal* crystal;
  Crystal* fitCrystal;

  QList<FitMarker> marker;
  QList<FitParameter*> fitParameters;
  QList<FitParameter*> baseParameters;

  Mat3D spotTransfer;
  Mat3D zoneTransfer;

};



#endif // FITDISPLAY_H
