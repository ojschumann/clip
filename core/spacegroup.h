#ifndef SPACEGROUP_H
#define SPACEGROUP_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QFlags>
#include <QRegExp>

class SpaceGroupCheck;

class SpaceGroup: public QObject
{
    Q_OBJECT
public:
    enum Systems {
        triclinic,
        monoclinic,
        orthorhombic,
        trigonal,
        hexagonal,
        tetragonal,
        cubic
    };
    Q_DECLARE_FLAGS(System, Systems)


    SpaceGroup(QObject*);
    bool setGroupSymbol(QString);

    QString groupSymbol() const;
    System crystalSystem() const;
    QList<int> getConstrains() const;


signals:
    void groupChanged();
    void constrainsChanged();
    void triclinHtoR();
    void triclinRtoH();


private:

    class SpaceGroupCheck {
    public:
        SpaceGroupCheck(SpaceGroup::System, QString, QString);
        bool match(QString);
        QStringList elements();

        SpaceGroup::System system;
        QString pointgroup;
        QRegExp regexp;
    };


    QString symbol;
    QStringList symbolElements;
    System crystalsystem;

    QList<SpaceGroupCheck> groups;

};






#endif // SPACEGROUP_H
