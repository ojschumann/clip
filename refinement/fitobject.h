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

#ifndef __FITOBJECT_H__
#define __FITOBJECT_H__

#include <QObject>
#include <QList>
#include <QVariant>

class FitParameter;
class FitParameterGroup;


class FitObject: public QObject {
  Q_OBJECT
public:
  FitObject(QObject* parent=0): QObject(parent) {}
  virtual ~FitObject() {}
  virtual void prepareForFit() {}
  virtual FitObject& operator=(const FitObject& o);
  void addParameterGroup(FitParameterGroup* g);
  virtual QString FitObjectName()=0;
  QList<FitParameter*> allParameters() const;
  QList<FitParameter*> changeableParameters() const;
  QList<FitParameter*> enabledParameters() const;
  virtual QList<FitObject*> getFitObjects();
signals:
  void fitObjectAdded(FitObject*);
  void fitObjectRemoved(FitObject*);
protected:
  QList<FitParameterGroup*> groups;

};

Q_DECLARE_METATYPE(FitObject*);

#endif
