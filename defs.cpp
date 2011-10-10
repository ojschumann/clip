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

#include "defs.h"
#include <cmath>


const char* HG_REPRO_ID = __HG_REPRO_ID__;
const char* HG_REPRO_REV = __HG_REPRO_REV__;
const char* HG_REPRO_DATE = __HG_REPRO_DATE__;
const char* BUILD_DATE = __DATE__ ;
const char* BUILD_TIME = __TIME__ ;



Mean::Mean(): N(0), M1(0), M2(0) {};
void Mean::add(double value) { N++; double oldM1 = M1; M1 += (value-M1)/N; M2 += (value-M1)*(value-oldM1); }
double Mean::mean() { return M1; }
double Mean::var() { return N>0 ? sqrt(M2/N) : 0.0; }
double Mean::unbiasedVar() { return N>1 ? sqrt(M2/(N-1)) : 0.0 ; }
