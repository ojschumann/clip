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

#ifndef TOOLS_H
#define TOOLS_H

class QWidget;
class QSizeF;

#include <QTransform>
#include <cmath>

static inline int ggt(int a, int b) {
  while (b) {
    int tb=b;
    b=a%b;
    a=tb;
  }
  if (a<0) return -a;
  return a;
}

static inline double fasthypot(double a, double b) {
  a = fabs(a);
  b = fabs(b);
  if (a>b) {
    b /= a;
    return a * sqrt(1.0+b*b);
  } else {
    a /= b;
    return b * sqrt(1.0+a*a);
  }
}

void setPaletteForStatus(QWidget* widget, bool ok);

QSizeF transformSize(const QSizeF& s, const QTransform& t);


#endif // TOOLS_H
