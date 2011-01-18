#include "indexparser.h"

#include <QRegExp>
#include <QStringList>


IndexParser::IndexParser(QString s) {
  text = s;
  valid = false;
  integral = false;

  QRegExp compactIndex("\\s*(\\d)(\\d)(\\d)\\s*");
  QRegExp normalIndex("\\s*(-?\\d+(?:\\.\\d+)?)\\s+(-?\\d+(?:\\.\\d+)?)\\s+(-?\\d+(?:\\.\\d+)?)\\s*");
  if (compactIndex.exactMatch(s)) {
    valid = true;
    integral = true;
    for (int i=0; i<3; i++)
      indexVector(i) = compactIndex.capturedTexts().at(i+1).toInt();
  } else if (normalIndex.exactMatch(s)) {
    valid = true;
    integral = true;
    for (int i=0; i<3; i++) {
      indexVector(i) = normalIndex.capturedTexts().at(i+1).toDouble();
      double t;
      integral = integral && (std::modf(indexVector(i), &t)<1e-4);
    }
  }
}

QString IndexParser::formatIndex(const Vec3D &index, int precision) {
  if (precision>0) {
    int minPrec=precision;
    for (int i=0; i<3; i++) {
      QString s = QString::number(index(i), 'f', precision);
      int j=0;
      while (j<s.size() && s.at(s.size()-1-j)=='0') j++;
      if (j<minPrec) minPrec = j;
    }
    precision -= minPrec;
  }
  QStringList l;
  for (int i=0; i<3; i++) {
    l << QString::number(index(i), 'f', precision);
  }
  if (l.at(0).size()==1 && l.at(1).size()==1 && l.at(2).size()==1) return l.join("");
  return l.join(" ");
}

Vec3D IndexParser::index() {
  return indexVector;
}

bool IndexParser::isValid() {
  return valid;
}

bool IndexParser::isIntegral() {
  return integral;
}
