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

#ifndef DEFS_H
#define DEFS_H

// Will be generated by make via call to HG
extern const char* HG_REPRO_ID;
extern const char* HG_REPRO_REV;
extern const char* HG_REPRO_DATE;
extern const char* BUILD_DATE;
extern const char* BUILD_TIME;

#define VER_FILEVERSION             4,0,0,2
#define VER_FILEVERSION_STR         "4.0.0.2\0"

#define VER_PRODUCTVERSION          4,0,0,2
#define VER_PRODUCTVERSION_STR      "4.0beta2\0"

#define VER_COMPANYNAME_STR         ""
#define VER_FILEDESCRIPTION_STR     "Software for evaluation of Laue diffraction images"
#define VER_INTERNALNAME_STR        "Cologne Laue Indexation Program"
#define VER_LEGALCOPYRIGHT_STR      "Copyright � 2008-2011 Olaf J. Schumann"
#define VER_LEGALTRADEMARKS1_STR    "Published under the termes of the Gnu Public License"
#define VER_LEGALTRADEMARKS2_STR    ""
#define VER_ORIGINALFILENAME_STR    "Clip.exe"
#define VER_PRODUCTNAME_STR         "Clip"

#define VER_COMPANYDOMAIN_STR       "clip.berlios.de"

static inline unsigned long long rdtsctime()
{
     unsigned int eax, edx;
     unsigned long long val;
     __asm__ __volatile__("cpuid": : : "ax", "bx", "cx", "dx");
     __asm__ __volatile__("rdtsc":"=a"(eax), "=d"(edx));
     val = edx;
     val = val << 32;
     val += eax;
     return val;
}

struct Mean {
  Mean();
  void add(double value);
  double mean();
  double var();
  double unbiasedVar();
  int N;
  double M1;
  double M2;
};


#endif // DEFS_H
