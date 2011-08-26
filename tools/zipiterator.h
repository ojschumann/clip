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

#ifndef ZIPITERATOR_H
#define ZIPITERATOR_H

#include <QPair>

template <class Container1, class Container2> class ZipIteratorClass {
public:
  ZipIteratorClass(class Container1::const_iterator i1, class Container2::const_iterator i2):
      c1iter(i1),
      c2iter(i2)
  {}
  bool operator !=(const ZipIteratorClass& o) const {
    return (c1iter!=o.c1iter) && (c2iter!=o.c2iter);
  }
  void operator ++() { c1iter++; c2iter++; }
  QPair<typename Container1::value_type, typename Container2::value_type> operator*() const { return qMakePair(*c1iter, *c2iter); }
private:
  typename Container1::const_iterator c1iter;
  typename Container2::const_iterator c2iter;
};

template <typename Container1, typename Container2> class ZipClass {
  friend class ZipIteratorClass<Container1, Container2>;
public:
  typedef ZipIteratorClass<Container1, Container2> const_iterator;

  explicit ZipClass(const Container1& _c1, const Container2& _c2): c1(_c1), c2(_c2) {};
  const_iterator begin() const { return ZipIteratorClass<Container1, Container2>(c1.begin(), c2.begin()); }
  const_iterator end() const { return ZipIteratorClass<Container1, Container2>(c1.end(), c2.end()); }
protected:
  const Container1& c1;
  const Container2& c2;
private:
};

template <typename Container1, typename Container2> ZipClass<Container1, Container2> Zip(const Container1& c1, const Container2& c2) {
  return ZipClass<Container1, Container2>(c1, c2);
}

#endif // ZIPITERATOR_H
