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
