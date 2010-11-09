#ifndef DEBUG_H
#define DEBUG_H

template<class T> void printRect(const T& r) {
  cout << "Rect = " << r.left() << " <-> " << r.right() << " | " << r.top() << " <-> " << r.bottom() << "  (" << r.width() << " x " << r.height() << ")" << endl;
}


#endif // DEBUG_H
