#ifndef INDEXPARSER_H
#define INDEXPARSER_H

#include <QString>
#include "tools/vec3D.h"

class IndexParser
{
public:
  IndexParser(QString);
  Vec3D index();
  bool isValid();
  bool isIntegral();
  static QString formatIndex(const Vec3D& index, int precision=3);
private:
  QString text;
  Vec3D indexVector;
  bool valid;
  bool integral;
};

#endif // INDEXPARSER_H
