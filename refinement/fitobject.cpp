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

#include "refinement/fitobject.h"

#include "refinement/fitparameter.h"





void FitObject::addParameterGroup(FitParameterGroup* g) {
  groups << g;
}

FitObject& FitObject::operator=(const FitObject& o) {
  QList<FitParameter*> tP = allParameters();
  QList<FitParameter*> oP = o.allParameters();
  for (int n=0; n<tP.size(); n++) {
    tP.at(n)->setChangeable(oP.at(n)->isChangeable());
    tP.at(n)->setEnabled(oP.at(n)->isEnabled());
  }
  return *this;
}

QList<FitParameter*> FitObject::allParameters() const {
  QList<FitParameter*>  list;
  foreach (FitParameterGroup* g, groups)
    list += g->parameters();
  return list;
}

QList<FitParameter*> FitObject::changeableParameters() const {
  QList<FitParameter*>  list;
  foreach (FitParameterGroup* g, groups)
    foreach (FitParameter* p, g->parameters())
      if (p->isChangeable())
        list += p;
  return list;
}

QList<FitParameter*> FitObject::enabledParameters() const {
  QList<FitParameter*>  list;
  foreach (FitParameterGroup* g, groups)
    foreach (FitParameter* p, g->parameters())
      if (p->isEnabled())
        list += p;
  return list;
}

QList<FitObject*> FitObject::getFitObjects() {
  return QList<FitObject*>() << this;
}
