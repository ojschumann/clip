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
