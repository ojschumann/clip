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

  double score();

  Ui::FitDisplay *ui;
  Crystal* crystal;
  Crystal* fitCrystal;

  QList<FitMarker> marker;
  QList<FitParameter*> fitParameters;
  QList<FitParameter*> baseParameters;

  Mat3D spotTransfer;
  Mat3D zoneTransfer;

};


template <int N, FitDisplay* fit> class Vertex {
public:
  Vertex();
  Vertex(const Vertex& o);
  double score;
  QVector<double> coordinates;

  bool operator<(const Vertex& o) const;
  Vertex& operator=(const Vertex& o);
  Vertex& operator+=(const Vertex& o);
  Vertex& operator-=(const Vertex& o);
  Vertex& operator*=(double scale);
  Vertex operator+(const Vertex& o) const;
  Vertex operator-(const Vertex& o) const;
  Vertex operator*(double scale) const;
};

#endif // FITDISPLAY_H
