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

#ifndef RESIZEINGTABLEWIDGET_H
#define RESIZEINGTABLEWIDGET_H

#include <QTableWidget>

#include "config.h"

class ResizingTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit ResizingTableWidget(QWidget* _parent = nullptr);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
signals:

public slots:

};

#endif // RESIZEINGTABLEWIDGET_H
