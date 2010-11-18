#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

using namespace std;

template<class T> void printRect(const T& r) {
  cout << "Rect = " << r.left() << " <-> " << r.right() << " | " << r.top() << " <-> " << r.bottom() << "  (" << r.width() << " x " << r.height() << ")" << endl;
}


template<class T> void printPoint(const QString& s, const T& r) {
  cout << qPrintable(s) << " = (" << r.x() << "," << r.y() << ")" << endl;
}


#endif // DEBUG_H
