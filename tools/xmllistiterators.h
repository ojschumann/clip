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

#ifndef XMLLISTITERATORS_H
#define XMLLISTITERATORS_H

#include <QList>
#include <QDomNode>
#include <QDomElement>
#include <QDomNodeList>


template <class C> class XmlIteratorClass {
public:
  XmlIteratorClass(int i, const C& _qdnl): pos(i), qdnl(_qdnl) {}
  bool operator !=(const XmlIteratorClass& o) const { return pos!=o.pos; }
  void operator ++() { pos++; }
  void operator+=(int n) { pos+=n; }
  typename C::element operator*() const { return qdnl.get(pos); }
  int operator-(XmlIteratorClass& o) const { return pos-o.pos; }
private:
  int pos;
  const C& qdnl;
};

class QDNLnodes {
  friend class XmlIteratorClass<QDNLnodes>;
public:
  typedef QDomNode element;
  typedef XmlIteratorClass<QDNLnodes> const_iterator;

  QDNLnodes(QDomNodeList l);
  const_iterator begin() const;
  const_iterator end() const;
protected:
  QDomNode get(int) const;
private:
  QDomNodeList list;
};

class QDNLelements {
  friend class XmlIteratorClass<QDNLelements>;
public:
  typedef QDomElement element;
  typedef XmlIteratorClass<QDNLelements> const_iterator;
  QDNLelements(QDomNodeList l);
  const_iterator begin() const;
  const_iterator end() const;
protected:
  QDomElement get(int) const;
private:
  QDomNodeList list;
  QList<int> elements;
};


#endif // XMLLISTITERATORS_H
