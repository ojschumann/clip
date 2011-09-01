/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

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
