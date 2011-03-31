#include "projectorfactory.h"



ProjectorFactory::ProjectorFactory(){
}

ProjectorFactory::ProjectorFactory(const ProjectorFactory &) {
}

ProjectorFactory::~ProjectorFactory() {
}

ProjectorFactory& ProjectorFactory::getInstance() {
  static ProjectorFactory instance;
  return instance;
}

Projector* ProjectorFactory::getProjector(QString key) {
  if (providers.contains(key)) {
    return providers[key]();
  }
  return NULL;
}

bool ProjectorFactory::registerProjector(QString key, Provider provider) {
  ProjectorFactory::getInstance().providers.insert(key, provider);
  return true;
}
