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

#include "simplergbscaler.h"
#include "image/datascalerfactory.h"
 

using namespace std;

SimpleRGBScaler::SimpleRGBScaler(DataProvider* dp, QObject* _parent) :
    DataScaler(dp, _parent)
{
  data = static_cast<QRgb const*>(dp->getData());
  datawidth = dp->size().width();
  dataheight = dp->size().height();
}

SimpleRGBScaler::SimpleRGBScaler(const SimpleRGBScaler &): DataScaler(0)  {}

SimpleRGBScaler::~SimpleRGBScaler() {
}

DataScaler* SimpleRGBScaler::getScaler(DataProvider *dp, QObject* _parent) {
  return new SimpleRGBScaler(dp, _parent);
}
#include <cmath>
QRgb SimpleRGBScaler::getRGB(const QPointF &p) {
  int x = static_cast<int>(std::floor(p.x()));
  int y = static_cast<int>(std::floor(p.y()));
  if (x<0 || x>=datawidth || y<0 || y>=dataheight) {
    return 0xFFFF0000;
  } else {
    return *(data+x+y*datawidth);
  }
}


bool SimpleRGBScalerRegistered = DataScalerFactory::registerDataScaler(DataProvider::RGB8Bit, &SimpleRGBScaler::getScaler);
