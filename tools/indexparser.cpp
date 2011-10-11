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

#include "indexparser.h"

#include <QRegExp>
#include <QStringList>


IndexParser::IndexParser(QString s) {
  text = s;
  valid = false;
  integral = false;

  QRegExp compactIndex("\\s*(\\d)(\\d)(\\d)\\s*");
  QRegExp normalIndex("\\s*(-?\\d+(?:\\.\\d+)?)\\s+(-?\\d+(?:\\.\\d+)?)\\s+(-?\\d+(?:\\.\\d+)?)\\s*");
  if (compactIndex.exactMatch(s)) {
    valid = true;
    integral = true;
    for (int i=0; i<3; i++)
      indexVector(i) = compactIndex.capturedTexts().at(i+1).toInt();
  } else if (normalIndex.exactMatch(s)) {
    valid = true;
    integral = true;
    for (int i=0; i<3; i++) {
      indexVector(i) = normalIndex.capturedTexts().at(i+1).toDouble();
      double t;
      integral = integral && (std::modf(indexVector(i), &t)<1e-4);
    }
  }
}

QString IndexParser::formatIndex(const Vec3D &index, int precision) {
  if (precision>0) {
    int minPrec=precision;
    for (int i=0; i<3; i++) {
      QString s = QString::number(index(i), 'f', precision);
      int j=0;
      while (j<s.size() && s.at(s.size()-1-j)=='0') j++;
      if (j<minPrec) minPrec = j;
    }
    precision -= minPrec;
  }
  QStringList l;
  for (int i=0; i<3; i++) {
    l << QString::number(index(i), 'f', precision);
  }
  if (l.at(0).size()==1 && l.at(1).size()==1 && l.at(2).size()==1) return l.join("");
  return l.join(" ");
}

Vec3D IndexParser::index() {
  return indexVector;
}

bool IndexParser::isValid() {
  return valid;
}

bool IndexParser::isIntegral() {
  return integral;
}
