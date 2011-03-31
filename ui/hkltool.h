#ifndef HKLTOOL_H
#define HKLTOOL_H

#include <QMainWindow>

namespace Ui {
  class HKLTool;
}

class Crystal;
class LiveMarkerModel;
class HKLTool : public QMainWindow
{
  Q_OBJECT

public:
  explicit HKLTool(Crystal* c, QWidget *parent = 0);
  ~HKLTool();
protected slots:
  void highlightMarkers();
  void highlightMarkerNr(int n);
  void deleteActiveMarker();
private:
  Ui::HKLTool *ui;
  LiveMarkerModel* markerModel;
};

#endif // HKLTOOL_H
