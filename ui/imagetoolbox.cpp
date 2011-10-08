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

#include "imagetoolbox.h"
#include "ui_imagetoolbox.h"

#include "image/laueimage.h"
#include "tools/cropmarker.h"
#include "ui/resolutioncalculator.h"
#include "ui/contrastcurves.h"
#include "core/projector.h"

ImageToolbox::ImageToolbox(Projector* p, QWidget* _parent):
    QMainWindow(_parent),
    ui(new Ui::ImageToolbox),
    projector(p)
{
  ui->setupUi(this);

  if (projector && projector->getLaueImage()) {
    connect(projector->getLaueImage(), SIGNAL(destroyed()), this, SLOT(deleteLater()));

    ui->tabWidget->addTab(new ContrastCurves(projector->getLaueImage()), "Contrast");
    ui->tabWidget->addTab(new ResolutionCalculator(projector->rulers(), projector->getLaueImage()), "Resolution");

    foreach (QWidget* page, projector->getLaueImage()->toolboxPages()) {
      ui->tabWidget->addTab(page, page->objectName());
    }
  } else {
    deleteLater();
  }
}

ImageToolbox::~ImageToolbox()
{
  delete ui;
}


#include <tools/debug.h>
