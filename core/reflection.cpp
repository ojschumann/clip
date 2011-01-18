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
