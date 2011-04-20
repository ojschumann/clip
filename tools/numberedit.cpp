#include "numberedit.h"

#include <QDebug>

NumberEdit::NumberEdit(QWidget *parent, bool _emptyValid) :
    QDoubleSpinBox(parent), emptyValid(_emptyValid)
{
}


QValidator::State NumberEdit::validate( QString & text, int & pos ) const {
  if (emptyValid && text.isEmpty()) return QValidator::Acceptable;
  QValidator::State state = QDoubleSpinBox::validate(text, pos);
  QChar decimalPoint = QLocale().decimalPoint();
  text.replace(QRegExp("[\\.,]"), decimalPoint);
  if (state == QValidator::Invalid && pos>0 && pos-1<text.size()) {
    QString s(text);
    bool changedString=false;

    // Stange thing entered...
    QChar justEntered = s[pos-1];
    if (justEntered!=decimalPoint && !justEntered.isDigit())
      return state;

    if (s.count(decimalPoint)>1) {
      // has multiple decimal points
      int p=0;
      while ((p=s.indexOf(decimalPoint, p))!=-1) {
        if (p+1!=pos) {
          if (p<pos) pos--;
          s.remove(p, 1);
          changedString = true;
        } else {
          p++;
        }
      }
    } else {
      // Split at decimal point
      QStringList N=s.split(decimalPoint);
      if (N.size()==2 and N[1].size()!=decimals()) {
        QString frac = N[1];
        while (frac.size()<decimals()) frac += "0";
        while (frac.size()>decimals()) frac.chop(1);
        s = N[0]+decimalPoint+frac;
        changedString = true;
      }
    }


    if (changedString) {
      state = validate(s, pos);
      if (state==QValidator::Acceptable) {
        text = s;
      }
    }
  }
  return state;
}
