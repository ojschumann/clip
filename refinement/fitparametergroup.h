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

#ifndef FITPARAMETERGROUP_H
#define FITPARAMETERGROUP_H

#include <QObject>

class FitParameter;
class FitObject;

class FitParameterGroup: public QObject {
  Q_OBJECT
public:
  FitParameterGroup(FitObject* o);
  virtual ~FitParameterGroup();
  QList<FitParameter*> parameters() const { return groupParameters; }
  void notifyCached() { valuesCached = true; }
  virtual void notifySetEnabled(int /* member */, bool /* b */) {}
  virtual void notifySetChangeable(int /* member */, bool /*b*/) {}
  void setValue();
  FitObject* getFitObject() { return fitObject; }
  virtual double value(int member) const = 0;
  virtual double epsilon(int member) const = 0;
  virtual double lowerBound(int member) const = 0;
  virtual double upperBound(int member) const = 0;
public slots:
  void groupDataChanged();
protected:
  void addParameter(QString name, bool initiallyEnabled=false);
  virtual void doSetValue(QList<double> values)=0;
  QList<FitParameter*> groupParameters;
  bool valuesCached;
  FitObject* fitObject;
};



#endif // FITPARAMETERGROUP_H
