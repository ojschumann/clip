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

#include "defs.h"
#include <Qt>

#define STR(s) #s
#define SSTR(s) STR(s)
#define DEFINE_HG_VARIABLE(name) const char* HG_REPRO_ ## name = strcmp(SSTR(CLIP_HG_REPRO_ ## name), STR(CLIP_HG_REPRO_ ## name)) ? SSTR(CLIP_HG_REPRO_ ## name) : "n/a";


//DEFINE_HG_VARIABLE(ID)
//DEFINE_HG_VARIABLE(REV)
//DEFINE_HG_VARIABLE(DATE)

const char* HG_REPRO_ID =
#ifdef CLIP_HG_REPRO_ID
    CLIP_HG_REPRO_ID;
#else
    "n/a";
#endif

const char* HG_REPRO_REV =
#ifdef CLIP_HG_REPRO_REV
    CLIP_HG_REPRO_REV;
#else
"n/a";
#endif

const char* HG_REPRO_DATE =
#ifdef CLIP_HG_REPRO_DATE
    CLIP_HG_REPRO_DATE;
#else
"n/a";
#endif

#define HH(x) #x
#define HHH(x) HH(x)
const char* QT_BUILD_VERSION = QT_VERSION_STR;
const char* QT_RUN_VERSION = qVersion();
const char* CXX_VERSION_STR =
#if defined __GNUC__
    #if defined _WIN32
    #  define GCC_NAME "MinGW "
    #else
    #  define GCC_NAME "gcc "
    #endif
    #define T GCC_NAME HHH(__GNUC__) "." HHH(__GNUC_MINOR__) "." HHH(__GNUC_PATCHLEVEL__)
    T;
#else
    "n/a";
#endif

const char* BUILD_DATE = __DATE__ ;
const char* BUILD_TIME = __TIME__ ;

