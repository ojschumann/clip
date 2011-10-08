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

#ifndef INDEXPARSER_H
#define INDEXPARSER_H

#include <QString>
#include "tools/vec3D.h"

class IndexParser
{
public:
  IndexParser(QString);
  Vec3D index();
  bool isValid();
  bool isIntegral();
  static QString formatIndex(const Vec3D& index, int precision=3);
private:
  QString text;
  Vec3D indexVector;
  bool valid;
  bool integral;
};

#endif // INDEXPARSER_H
