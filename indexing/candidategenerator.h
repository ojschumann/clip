#ifndef CANDIDATEGENERATOR_H
#define CANDIDATEGENERATOR_H

#include <QObject>
#include <QList>

#include "tools/mat3D.h"

class CandidateGenerator: public QObject
{
  Q_OBJECT
public:
    CandidateGenerator();
    TVec3D<int> getNextCandidate();
    void reset();
signals:
    void nextN(int);
private:
    void addToGroup(const TMat3D<int>&);
    QList< TMat3D<int> > group;

};

#endif // CANDIDATEGENERATOR_H
