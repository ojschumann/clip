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

#ifndef INDEXDISPLAY_H
#define INDEXDISPLAY_H

#include <QWidget>

#include "indexing/solutionmodel.h"

namespace Ui {
    class Indexing;
}

class Crystal;

class IndexDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit IndexDisplay(Crystal* _c, QWidget* _parent = nullptr);
    virtual ~IndexDisplay();
    int maxSearchIndex();
signals:
    void stopIndexer();
    void maxSearchIndexChanged(int);
private:
    Ui::Indexing *ui;
    Crystal* crystal;
    SolutionModel solutions;

    bool indexRunning;

private slots:
    void on_startButton_clicked();
    void updateSolutionDisplay(QModelIndex, QModelIndex);
    void indexerDestroyed();
    void showMajorIndex(int);
    void showNumberOfSolutions(int);
    void setProgress(int);
};

#endif // INDEXDISPLAY_H
