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

#ifndef PROPAGATINGGRAPHICSOBJECT_H
#define PROPAGATINGGRAPHICSOBJECT_H

#include <QGraphicsObject>
#include <QVariant>

class PropagatingGraphicsObject : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit PropagatingGraphicsObject(QGraphicsItem *parent = 0);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void setImgTransform(const QTransform&);
protected:
    bool propagatePositionChange;
    bool propagateTransformChange;
    QTransform propagatedTransform;

signals:

public slots:

};

#endif // PROPAGATINGGRAPHICSOBJECT_H
