/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

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

#ifndef FITPARAMETER_H
#define FITPARAMETER_H

#include <QObject>
#include <QTreeWidgetItem>

#include "refinement/fitparametergroup.h"

class FitParameter: public QObject {
  Q_OBJECT
public:
  FitParameter(QString n, int id, FitParameterGroup& g);
  virtual ~FitParameter() {}
  QString name() const { return _name; }
  double value() const { return group.value(memberId); }
  double error() const { return errorValue; }
  void prepareValue(double);
  double getCachedValue() const;
  FitParameterGroup* getGroup() { return &group; }
  FitObject* getFitObject() { return group.getFitObject(); }

  void setValue(double e=-1) { errorValue=e; group.setValue(); hasCachedValue=false; emitValueChanged(); }
  double lowerBound() const { return group.lowerBound(memberId); }
  double upperBound() const { return group.upperBound(memberId); }
  double epsilon() const { return group.epsilon(memberId); }
  bool isEnabled() const { return enabled && changeable; }
  void setEnabled(bool b) {enabled = b; errorValue = -1; emit enabledStateChanged(this, b); group.notifySetEnabled(memberId, b);}
  bool isChangeable() const { return changeable; }
  void setChangeable(bool b) { changeable=b; errorValue = -1; emit changeableStateChanged(this, b); group.notifySetChangeable(memberId, b);}
public slots:
  void emitValueChanged() { emit valueChanged(this, value()); }
signals:
  void valueChanged(FitParameter*, double);
  void enabledStateChanged(FitParameter*, bool);
  void changeableStateChanged(FitParameter*, bool);
protected:
  QString _name;
  int memberId;
  bool enabled;
  bool changeable;
  FitParameterGroup& group;
  double errorValue;
  double cachedValue;
  bool hasCachedValue;
};


#endif // FITPARAMETER_H
