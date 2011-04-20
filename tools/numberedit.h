#ifndef NUMBEREDIT_H
#define NUMBEREDIT_H

#include <QDoubleSpinBox>

class NumberEdit : public QDoubleSpinBox
{
  Q_OBJECT
public:
  explicit NumberEdit(QWidget *parent = 0, bool _emptyValid = false);
  virtual QValidator::State validate( QString & text, int & pos ) const;
signals:

public slots:
private:
  bool emptyValid;

};

#endif // NUMBEREDIT_H
