/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#ifndef CONTRASTCURVES_H
#define CONTRASTCURVES_H

#include <QWidget>
#include <QGraphicsScene>
#include <QPointer>

#include "image/laueimage.h"
#include "tools/circleitem.h"

class LaueImage;
class BezierCurve;
class ObjectStore;
class HistogramItem;

namespace Ui {
    class ContrastCurves;
}

class ContrastCurves : public QWidget
{
    Q_OBJECT

public:
    explicit ContrastCurves(LaueImage* img, QWidget *parent = 0);
    virtual ~ContrastCurves();
    void mousePressEvent(QMouseEvent *);
  public slots:
    void loadFromFile(const QString&);
    void saveToFile(const QString&);
    void newMarker(const QPointF&);
    void changeToCurve(int);
    void markerChanged();
    void updateCurveLines(int);
    void makeScales();
    void makeHScale();
    void makeVScale();
  private:
    class BoundedEllipse: public CircleItem {
    public:
      BoundedEllipse(QGraphicsItem *parent=0);
      QVariant itemChange(GraphicsItemChange change, const QVariant & value);
      void setBBox(const QRectF& b) { bbox = b; }
      bool operator<(const BoundedEllipse& o);
    private:
      QRectF bbox;
    };

    Ui::ContrastCurves *ui;
    QPointer<LaueImage> laueImage;
    QGraphicsScene scene;
    QList<BoundedEllipse*> handleMarkers;
    QList<QGraphicsPathItem*> curves;
    int activeCurve;
    HistogramItem* histogram;


private slots:
    void on_defaultCurveSelector_activated(int index);
    void on_saveButton_clicked();
    void on_loadButton_clicked();
};

#endif // CONTRASTCURVES_H
