/**************************************************************************
  Copyright (C) 2011 schumann

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
**************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#ifndef nullptr
#  define nullptr NULL
#endif

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(4,7,0)
#  define SCENEBLOWUP(x) ((x)*1000.0)
#  define SCENECOMPRESS(x) ((x)/1000.0)
#else
#  define SCENEBLOWUP(x) (x)
#  define SCENECOMPRESS(x) (x)
#endif


// Eigen lib has a bug concerning the stack alignment on Win32 with MinGW reported prior to 4.5
// see http://eigen.tuxfamily.org/dox/TopicWrongStackAlignment.html
// but MinGW 4.6.1 still has this issue
#if defined _WIN32 && defined __GNUC__
#define CLIP_EIGEN_STACK_ALIGN __attribute__((force_align_arg_pointer))
#endif

#endif // CONFIG_H
