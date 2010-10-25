#ifndef __OBJECTSTORE_H__
#define __OBJECTSTORE_H__

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QList>

class ObjectStore: public QObject {
    Q_OBJECT
    public:
        ObjectStore(QObject* parent=0);
        int size();
        QObject* at(int i);
        
    public slots:
        void addObject(QObject *o);
        void removeObject(QObject *o);
    signals:
        void objectAdded(QObject *o=0);
        void objectRemoved(QObject *o=0);
    private:
        QList<QObject *> set;
};

#endif
