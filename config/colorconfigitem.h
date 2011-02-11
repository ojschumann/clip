#ifndef COLORCONFIGITEM_H
#define COLORCONFIGITEM_H

#include <QObject>
#include <QString>
#include <QColor>

class ColorConfigItem: public QObject {
  Q_OBJECT
public:
  ColorConfigItem(QString name, QColor defaultColor, QObject* parent=0);
  ~ColorConfigItem();
  QColor color() const;
  QString name() const;
public slots:
  void setColor(const QColor& c);
signals:
  void colorChanged(QColor);
private:
  QColor _color;
  QString _name;
};


#endif // COLORCONFIGITEM_H
