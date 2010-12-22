#ifndef PROJECTORFACTORY_H
#define PROJECTORFACTORY_H

#include <QString>
#include <QMap>

class Projector;

class ProjectorFactory
{
public:
    typedef Projector*(*Provider)();

    static ProjectorFactory& getInstance();
    static bool registerProjector(QString, Provider);

    Projector* getProjector(QString);

  private:
    explicit ProjectorFactory();
    ProjectorFactory(const ProjectorFactory&);
    ~ProjectorFactory();

    QMap<QString, Provider> providers;

};

#endif // PROJECTORFACTORY_H
