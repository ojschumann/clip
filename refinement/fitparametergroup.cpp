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

#include "fitparametergroup.h"

#include "refinement/fitparameter.h"


FitParameterGroup::FitParameterGroup(FitObject *o):
    fitObject(o)
{
  valuesCached = false;
}

FitParameterGroup::~FitParameterGroup()  {
  foreach (FitParameter* p, groupParameters)
    delete p;
  groupParameters.clear();
}

void FitParameterGroup::setValue() {
  if (valuesCached) {
    QList<double> values;
    foreach(FitParameter* p, groupParameters)
      values +=p->getCachedValue();
    doSetValue(values);
  }
  valuesCached=false;
}

void FitParameterGroup::addParameter(QString name, bool initiallyEnabled)  {
  groupParameters += new FitParameter(name, groupParameters.size(), *this);
  groupParameters.last()->setEnabled(initiallyEnabled);
}

void FitParameterGroup::groupDataChanged() {
  foreach (FitParameter* p, groupParameters) {
    p->emitValueChanged();
  }
}
