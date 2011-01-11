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
