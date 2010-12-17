#include <tools/objectstore.h>

ObjectStore::ObjectStore(QObject* parent): QObject(parent), set() {
}
int ObjectStore::size() {
    return set.size();
}


QObject* ObjectStore::at(int i) {
    if (i<size()) 
        return set[i];
    else
        return NULL;
}
        
void ObjectStore::addObject(QObject *o) {
    connect(o, SIGNAL(destroyed(QObject *)), this, SLOT(removeObject(QObject*)));
    set.append(o);
    emit objectAdded(o);
}

void ObjectStore::removeObject(QObject *o) {
    set.removeAll(o);
    emit objectRemoved(o);
}
