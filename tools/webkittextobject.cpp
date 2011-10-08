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
#include <QSharedPointer>
#include <QCache>
#include <QPicture>
#include <QPair>
#include <QWebSettings>
#include <QWebPage>
#include <QWebFrame>

typedef QCache<QPair<int, QString>, QPicture> PictureCacheClass;



WebkitTextObject::WebkitTextObject(QObject* _parent): QObject(_parent)  {
}

WebkitTextObject::~WebkitTextObject()  {
}

QSizeF WebkitTextObject::intrinsicSize(QTextDocument* /*doc*/, int /*posInDocument*/,
                                    const QTextFormat &format)
{
  return getPicture(format)->boundingRect().size();
}

void WebkitTextObject::drawObject(QPainter *painter, const QRectF &rect,
                               QTextDocument * /*doc*/, int /*posInDocument*/,
                               const QTextFormat &format)
{
  painter->drawPicture(rect.topLeft(), *getPicture(format));
}

QPicture* WebkitTextObject::getPicture(const QTextFormat &format) {
  int fontSize = (format.hasProperty(QTextCharFormat::FontPointSize)) ?
                 format.property(QTextCharFormat::FontPointSize).toInt() :
                 QWebSettings::globalSettings()->fontSize(QWebSettings::DefaultFontSize);

  QString fontFamily = (format.hasProperty(QTextCharFormat::FontFamily)) ?
                       format.property(QTextCharFormat::FontFamily).toString() :
                       QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont);

  QPair<int, QString> key = qMakePair(fontSize, fontFamily);

  PictureCacheClass* cache = qVariantValue< QSharedPointer<PictureCacheClass> >(format.property(PictureCache)).data();

  if (!cache->contains(key)) {

    QWebPage* page = new QWebPage;
    page->mainFrame()->setHtml(format.property(HtmlString).toString());
    page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    page->mainFrame()->setZoomFactor(4.0/3.0); // No idea, why a zoom factor is necessary
    page->settings()->setFontSize(QWebSettings::DefaultFontSize, fontSize);
    page->settings()->setFontFamily(QWebSettings::StandardFont, fontFamily);

    page->setViewportSize(QSize(1,1));
    QSize s = page->mainFrame()->contentsSize();
    page->setViewportSize(s);


    QPicture* picture = new QPicture();

    QPainter painter(picture);
    page->mainFrame()->render(&painter, QWebFrame::ContentsLayer, QRegion(0, 0, s.width(), s.height()));
    painter.end();

    cache->insert(key, picture);
  }
  return cache->object(key);

}


void WebkitTextObject::registerObject(QTextDocument* doc) {
  doc->documentLayout()->registerHandler(WebkitTextFormat, new WebkitTextObject(doc));
}


void WebkitTextObject::insertObject(QTextEdit* edit, const QString& html) {
  QTextCursor cursor = edit->textCursor();

  QTextCharFormat webkitCharFormat;
  webkitCharFormat.setFont(edit->document()->defaultFont());
  webkitCharFormat.setObjectType(WebkitTextFormat);
  webkitCharFormat.setProperty(HtmlString, html);

  webkitCharFormat.setProperty(PictureCache, qVariantFromValue(QSharedPointer<PictureCacheClass>(new PictureCacheClass(10))));

  cursor.insertText(QString(QChar::ObjectReplacementCharacter), webkitCharFormat);
  edit->setTextCursor(cursor);
}

Q_DECLARE_METATYPE(QSharedPointer<PictureCacheClass>);
