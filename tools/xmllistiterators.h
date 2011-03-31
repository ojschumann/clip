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
