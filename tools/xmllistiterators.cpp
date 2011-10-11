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

#include "xmllistiterators.h"


QDNLnodes::QDNLnodes(QDomNodeList l):
    list(l)
{
}

QDNLnodes::const_iterator QDNLnodes::begin() const {
  return const_iterator(0, *this);
}

QDNLnodes::const_iterator QDNLnodes::end() const {
  return const_iterator(list.size(), *this);
}

QDomNode QDNLnodes::get(int pos) const {
  return list.at(pos);
}


// -------------------------------------------

QDNLelements::QDNLelements(QDomNodeList l):
    list(l)
{
  for (int i=0; i<list.size(); i++) {
    if (list.at(i).isElement()) {
      elements << i;
    }
  }
}

QDNLelements::const_iterator QDNLelements::begin() const {
  return const_iterator(0, *this);
}

QDNLelements::const_iterator QDNLelements::end() const {
  return const_iterator(elements.size(), *this);
}

QDomElement QDNLelements::get(int pos) const {
  return list.at(elements.at(pos)).toElement();
}
