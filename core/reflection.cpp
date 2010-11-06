#include <core/reflection.h>
#include <QString>

QString Reflection::formatOverline(int i) {
  if (i<0)
    return QString("<span style=""text-decoration:overline"">%1</span>").arg(-i);
  return QString("<span>%1</span>").arg(i);
}

QString Reflection::hkl2text(int h, int k, int l) {
  if (h<10 and k<10 and l<10) {
    return formatOverline(h)+formatOverline(k)+formatOverline(l);
  } else {
    return formatOverline(h)+" "+formatOverline(k)+" "+formatOverline(l);
  }
}

QString Reflection::toText() const {
  return hkl2text(h,k,l);
}
