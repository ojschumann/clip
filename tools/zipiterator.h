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
