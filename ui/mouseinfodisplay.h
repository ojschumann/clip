/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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

/* The flow of data to this display is somewhat difficult to trace. Here is a description:
   most signals are routed through the CLIP-instance.
   On a mouse move and a mouse leave event, the ProjectionPlane does a "emit mousePositionInfo(MousePositionInfo);"
   All plane are connected to the signal Clip::mousePositionInfo(MousePositionInfo); which is in turn connected to
   MouseInfoDisplay::showMouseInfo(MousePositionInfo).

   On call of showMouseInfo(MousePositionInfo), the MouseInfoDisplay connects the Projector::spotHighlightChanged(Vec3D, QString) slot
   to the MouseInfoDisplay::receiveSpotHightlight(Vec3D, QString) signal. An existing connection to this slot is disconnected first.
   Thus only the last Projector, that had a mousePositionInfo posted is connected by this signal.


   ProjectionPlane::mousePositionInfo(MousePositionInfo)
   -> Clip::mousePositionInfo(MousePositionInfo)
   -> MouseInfoDisplay::showMouseInfo(MousePositionInfo)
      connect(info.projector::spotHighlightChanged(Vec3D, QString) -> MouseInfoDisplay::receiveSpotHightlight(Vec3D, QString))
      emit MouseInfoDisplay::highlightMarker(Vec3D index)
      update Cursor section

   MouseInfoDisplay::highlightMarker(Vec3D index)
   -> Clip::highlightMarker(Vec3D)
   -> Projector::setSpotHighlighting(Vec3D)
      updateSpotHighlightMarker()  // Also on Projector::project()
        emit Projector::spotHighlightChanged(hkl2Reziprocal(index))
         -> MouseInfoDisplay::receiveSpotHightlight(Vec3D, QString))

   */

class MouseInfoDisplay : public QWidget
{
  Q_OBJECT

public:
  explicit MouseInfoDisplay(QWidget *parent = 0);
  virtual ~MouseInfoDisplay();
  virtual bool eventFilter(QObject *, QEvent *);
  virtual QSize sizeHint() const;
signals:
  void highlightMarker(Vec3D);
public slots:
  void showMouseInfo(MousePositionInfo);
  void receiveSpotHightlight(Vec3D, QString);
protected:
  virtual void changeEvent(QEvent *);
private slots:
  void on_reflex_textEdited(QString );
  void cursorTableVisiblyToggled(bool b);
private:
  void doEmitHighlightMarker(const Vec3D& v, bool force=false);

  Ui::MouseInfoDisplay *ui;
  bool parentNeedSizeConstrain;
  Vec3D lastVector;
  QPointer<QObject> lastSender;
};

#endif // MOUSEINFODISPLAY_H
