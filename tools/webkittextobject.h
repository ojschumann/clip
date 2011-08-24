/**************************************************************************
  Copyright (C) 2011 schumann

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
**************************************************************************/

#ifndef WEBKITTEXTOBJECT_H
#define WEBKITTEXTOBJECT_H

#include <QObject>
#include <QTextObjectInterface>
#include <QTextFormat>

class QTextDocument;
class QTextFormat;
class QPainter;
class QRectF;
class QSizeF;
class QTextEdit;

class WebkitTextObject : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    enum { WebkitTextFormat = QTextFormat::UserObject + 1 };
    enum WebkitProperties { WebPage = 1, TestClass = 2 };

    WebkitTextObject(QObject* parent=0);
    virtual ~WebkitTextObject();

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format);

    static void registerObject(QTextDocument* doc);
    static void insertObject(QTextEdit* edit, const QString& html);

};

#endif // WEBKITTEXTOBJECT_H
