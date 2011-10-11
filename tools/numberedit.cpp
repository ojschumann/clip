/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

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

#include "numberedit.h"

NumberEdit::NumberEdit(QWidget* _parent, bool _emptyValid) :
    QDoubleSpinBox(_parent), emptyValid(_emptyValid)
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
