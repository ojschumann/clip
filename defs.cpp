#include "defs.h"


const char* HG_REPRO_ID = __HG_REPRO_ID__;
const char* HG_REPRO_REV = __HG_REPRO_REV__;
const char* HG_REPRO_DATE = __HG_REPRO_DATE__;
const char* BUILD_DATE = __DATE__ ;
const char* BUILD_TIME = __TIME__ ;

#include <QRegExp>
#include <QStringList>
#include <iostream>

using namespace std;

Vec3D parseHKLString(const QString& s, bool* ok) {
  QRegExp hklmatch("\\s*(?:(-?\\d)(-?\\d)(-?\\d))|(?:(-?\\d+(?:\\.\\d+)?)\\s+(-?\\d+(?:\\.\\d+)?)\\s+(-?\\d+(?:\\.\\d+)?))\\s*");
  if (hklmatch.exactMatch(s)) {
    if (ok) *ok = true;
    foreach (QString t, hklmatch.capturedTexts()) {
      cout << qPrintable(t) << endl;
    }
    return Vec3D(1,1,1);
  }
  if (ok) *ok = false;
  return Vec3D();
}
