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

#include <QApplication>
#include <QTime>
#include <cmath>
 
#include "ui/clip.h"

using namespace std;

#ifdef CLIP_STATIC
#include <QtPlugin>
#  if QT_VERSION<QT_VERSION_CHECK(4,7,0)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qtiff)
Q_IMPORT_PLUGIN(qmng)
#  endif
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qico)
Q_IMPORT_PLUGIN(qsvg)
#endif

#include "config.h"


int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  a.setApplicationName("Clip");
  a.setOrganizationDomain("clip4.sf.net");
  a.setOrganizationName("O.J.Schumann");

  Clip* w = Clip::getInstance();
  w->show();
  int r = a.exec();
  Clip::clearInstance();
  return r;
}


