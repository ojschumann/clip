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

#include "webkittextobject.h"


#include <QtGui>
#include <QtWebKit>
#include <QSharedPointer>
#include <QDebug>

WebkitTextObject::WebkitTextObject(QObject* parent): QObject(parent)  {
}

WebkitTextObject::~WebkitTextObject()  {
}

QSizeF WebkitTextObject::intrinsicSize(QTextDocument * doc, int /*posInDocument*/,
                                    const QTextFormat &format)
{
  static int calls = 0;
  QWebPage* page = qVariantValue< QSharedPointer<QWebPage> >(format.property(WebPage)).data();

  if (format.hasProperty(QTextCharFormat::FontPointSize))
    page->settings()->setFontSize(QWebSettings::DefaultFontSize, format.property(QTextCharFormat::FontPointSize).toInt());
  if (format.hasProperty(QTextCharFormat::FontFamily))
    page->settings()->setFontFamily(QWebSettings::StandardFont, format.property(QTextCharFormat::FontFamily).toString());
  page->setViewportSize(QSize(1,1));
  page->setViewportSize(page->mainFrame()->contentsSize());
  qDebug() << calls++ << page->mainFrame()->contentsSize() << page->mainFrame()->zoomFactor() << page->mainFrame()->textSizeMultiplier();
  return QSizeF(page->mainFrame()->contentsSize());
}

void WebkitTextObject::drawObject(QPainter *painter, const QRectF &rect,
                               QTextDocument * /*doc*/, int /*posInDocument*/,
                               const QTextFormat &format)
{
  QWebPage* page = qVariantValue< QSharedPointer<QWebPage> >(format.property(WebPage)).data();
  QSize s = page->mainFrame()->contentsSize();
  painter->translate(rect.topLeft());
  page->mainFrame()->render(painter, QWebFrame::ContentsLayer, QRegion(0, 0, s.width(), s.height()));
}



void WebkitTextObject::registerObject(QTextDocument* doc) {
  doc->documentLayout()->registerHandler(WebkitTextFormat, new WebkitTextObject(doc));
}

class textclass {
public:
  textclass() {
    qDebug() << "Testclass init";
  }
  ~textclass() {
    qDebug() << "Testclass destructor";
  }
};

void WebkitTextObject::insertObject(QTextEdit* edit, const QString& html) {
  QWebPage* page = new QWebPage;
  page->mainFrame()->setHtml(html);
  page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
  page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
  page->mainFrame()->setZoomFactor(4.0/3.0); // No idea, why a zoom factor is necessary

  QTextCursor cursor = edit->textCursor();
  QTextCharFormat webkitCharFormat;
  webkitCharFormat = cursor.blockCharFormat();
  webkitCharFormat = cursor.charFormat();
  webkitCharFormat.setFont(edit->document()->defaultFont());
  webkitCharFormat.setObjectType(WebkitTextFormat);
  webkitCharFormat.merge(cursor.blockCharFormat());
  webkitCharFormat.merge(cursor.charFormat());
  webkitCharFormat.setProperty(WebPage, qVariantFromValue(QSharedPointer<QWebPage>(page)));
  webkitCharFormat.setProperty(TestClass, qVariantFromValue(QSharedPointer<textclass>(new textclass())));


  cursor.insertText(QString(QChar::ObjectReplacementCharacter), webkitCharFormat);
  edit->setTextCursor(cursor);
}

Q_DECLARE_METATYPE(QSharedPointer<QWebPage>);
Q_DECLARE_METATYPE(QSharedPointer<textclass>);
