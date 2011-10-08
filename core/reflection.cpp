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

#include <core/reflection.h>
#include <QString>

QString Reflection::formatOverline(int i) {
  if (i<0)
    return QString("<span style=""text-decoration:overline"">%1</span>").arg(-i);
  return QString("<span>%1</span>").arg(i);
}

QString Reflection::hkl2text(int h, int k, int l) {
  QString sep;
  if (abs(h)>=10 or abs(k)>=10 or abs(l)>=10)
    sep = " ";
  return formatOverline(h)+sep+formatOverline(k)+sep+formatOverline(l);
}

QString Reflection::toText() const {
  QString sep;
  if (h<0 or h>9 or k<0 or k>9 or l<0 or l>9)
    sep = " ";
  return QString::number(h)+sep+QString::number(k)+sep+QString::number(l);
}

QString Reflection::toHtml() const {
  return hkl2text(h,k,l);
}
