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

#include "solutionscorer.h"

#include "core/crystal.h"
#include "core/projector.h"
#include "core/projectorfactory.h"

SolutionScorer::SolutionScorer(QObject *parent) :
    QObject(parent),
    copiedCrystal(0)
{
}

SolutionScorer::~SolutionScorer() {
  deleteCrystal();
}

void SolutionScorer::copyCrystal(Crystal *c) {
  if (copiedCrystal)
    deleteCrystal();
  copiedCrystal = new Crystal();
  copiedCrystal->synchronUpdate(true);
  copiedCrystal->enableUpdate(true);
  *copiedCrystal = *c;

  foreach (Projector* p, c->getConnectedProjectors()) {
    if (p->hasMarkers()) {
      Projector* copyP = ProjectorFactory::getInstance().getProjector(p->projectorName());
      *copyP = *p;
      copyP->connectToCrystal(copiedCrystal);
    }
  }
}

void SolutionScorer::deleteCrystal() {
  if (!copiedCrystal) return;
  foreach (Projector* p, copiedCrystal->getConnectedProjectors()) {
    delete p;
  }
  delete copiedCrystal;
  copiedCrystal = 0;
}

void SolutionScorer::scoreSolution(Solution s) {
  s.indexMean = 0.0;
  s.indexRMS = 0.0;
  copiedCrystal->setRotation(s.bestRotation);
  cout << copiedCrystal->reflectionCount() << endl;
  foreach (AbstractMarkerItem* marker, copiedCrystal->getMarkers()) {
    //s.angularDeviation += marker->getAngularDeviation();
    //s.detectorPositionDeviation += marker->getDetectorPositionScore();
    TVec3D<int> index = marker->getIntegerIndex();
    s.indexMean += abs(index.x()) + abs(index.y()) + abs(index.z());
    s.indexRMS += 1.0*index.norm_sq();
  }
  s.indexMean /= 3.0*copiedCrystal->getMarkers().size();
  s.indexRMS /= 3.0*copiedCrystal->getMarkers().size();
  s.indexRMS = sqrt(s.indexRMS);

  emit solutionScored(s);
}
