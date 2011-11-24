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

#include "core/spacegroup.h"

QList<Spacegroup::SpacegroupSymbolInfo> Spacegroup::static_init() {
  // List of all possible Space group symbols from http://cci.lbl.gov/sginfo/hall_symbols.html
  // plus the following points
  // All centering symbols for  P1 and P-1, e.g. I1
  // Short symbols for monoclinic spacegroups with special axis b, e.g. P2 instead of P121
  // Cubic spacegroups with point groups m3 and m3m without the minus sign on 3, e.g. Pm3m instead of Pm-3m
  // F4/mmm as equivalent to I4/mmm

  return QList<SpacegroupSymbolInfo>()
   << SpacegroupSymbolInfo(1, "", "P 1", "", "P 1")
   << SpacegroupSymbolInfo(1, "", "A 1", "", "A 1")
   << SpacegroupSymbolInfo(1, "", "B 1", "", "B 1")
   << SpacegroupSymbolInfo(1, "", "C 1", "", "C 1")
   << SpacegroupSymbolInfo(1, "", "I 1", "", "I 1")
   << SpacegroupSymbolInfo(1, "", "F 1", "", "F 1")

   << SpacegroupSymbolInfo(2, "", "P -1", "", "-P 1")
   << SpacegroupSymbolInfo(2, "", "A -1", "", "-A 1")
   << SpacegroupSymbolInfo(2, "", "B -1", "", "-B 1")
   << SpacegroupSymbolInfo(2, "", "C -1", "", "-C 1")
   << SpacegroupSymbolInfo(2, "", "I -1", "", "-I 1")
   << SpacegroupSymbolInfo(2, "", "F -1", "", "-F 1")

   << SpacegroupSymbolInfo(3, "", "P 2", "", "P 2y")
   << SpacegroupSymbolInfo(3, "b", "P 1 2 1", "", "P 2y")
   << SpacegroupSymbolInfo(3, "c", "P 1 1 2", "", "P 2")
   << SpacegroupSymbolInfo(3, "a", "P 2 1 1", "", "P 2x")

   << SpacegroupSymbolInfo(4, "", "P 21", "", "P 2yb")
   << SpacegroupSymbolInfo(4, "b", "P 1 21 1", "", "P 2yb")
   << SpacegroupSymbolInfo(4, "c", "P 1 1 21", "", "P 2c")
   << SpacegroupSymbolInfo(4, "a", "P 21 1 1", "", "P 2xa")

   << SpacegroupSymbolInfo(5, "", "C 2", "", "C 2y")
   << SpacegroupSymbolInfo(5, "", "A 2", "", "A 2y")
   << SpacegroupSymbolInfo(5, "", "I 2", "", "I 2y")
   << SpacegroupSymbolInfo(5, "b1", "C 1 2 1", "", "C 2y")
   << SpacegroupSymbolInfo(5, "b2", "A 1 2 1", "", "A 2y")
   << SpacegroupSymbolInfo(5, "b3", "I 1 2 1", "", "I 2y")
   << SpacegroupSymbolInfo(5, "c1", "A 1 1 2", "", "A 2")
   << SpacegroupSymbolInfo(5, "c2", "B 1 1 2", "", "B 2")
   << SpacegroupSymbolInfo(5, "c3", "I 1 1 2", "", "I 2")
   << SpacegroupSymbolInfo(5, "a1", "B 2 1 1", "", "B 2x")
   << SpacegroupSymbolInfo(5, "a2", "C 2 1 1", "", "C 2x")
   << SpacegroupSymbolInfo(5, "a3", "I 2 1 1", "", "I 2x")

   << SpacegroupSymbolInfo(6, "", "P m", "", "P -2y")
   << SpacegroupSymbolInfo(6, "b", "P 1 m 1", "", "P -2y")
   << SpacegroupSymbolInfo(6, "c", "P 1 1 m", "", "P -2")
   << SpacegroupSymbolInfo(6, "a", "P m 1 1", "", "P -2x")
   << SpacegroupSymbolInfo(7, "b1", "P 1 c 1", "", "P -2yc")
   << SpacegroupSymbolInfo(7, "b1", "P 1 c 1", "", "P -2yc")
   << SpacegroupSymbolInfo(7, "b1", "P 1 c 1", "", "P -2yc")

   << SpacegroupSymbolInfo(7, "", "P c", "", "P -2yc")
   << SpacegroupSymbolInfo(7, "", "P n", "", "P -2yac")
   << SpacegroupSymbolInfo(7, "", "P a", "", "P -2ya")
   << SpacegroupSymbolInfo(7, "b1", "P 1 c 1", "", "P -2yc")
   << SpacegroupSymbolInfo(7, "b2", "P 1 n 1", "", "P -2yac")
   << SpacegroupSymbolInfo(7, "b3", "P 1 a 1", "", "P -2ya")
   << SpacegroupSymbolInfo(7, "c1", "P 1 1 a", "", "P -2a")
   << SpacegroupSymbolInfo(7, "c2", "P 1 1 n", "", "P -2ab")
   << SpacegroupSymbolInfo(7, "c3", "P 1 1 b", "", "P -2b")
   << SpacegroupSymbolInfo(7, "a1", "P b 1 1", "", "P -2xb")
   << SpacegroupSymbolInfo(7, "a2", "P n 1 1", "", "P -2xbc")
   << SpacegroupSymbolInfo(7, "a3", "P c 1 1", "", "P -2xc")

   << SpacegroupSymbolInfo(8, "", "C m", "", "C -2y")
   << SpacegroupSymbolInfo(8, "", "A m", "", "A -2y")
   << SpacegroupSymbolInfo(8, "", "I m", "", "I -2y")
   << SpacegroupSymbolInfo(8, "b1", "C 1 m 1", "", "C -2y")
   << SpacegroupSymbolInfo(8, "b2", "A 1 m 1", "", "A -2y")
   << SpacegroupSymbolInfo(8, "b3", "I 1 m 1", "", "I -2y")
   << SpacegroupSymbolInfo(8, "c1", "A 1 1 m", "", "A -2")
   << SpacegroupSymbolInfo(8, "c2", "B 1 1 m", "", "B -2")
   << SpacegroupSymbolInfo(8, "c3", "I 1 1 m", "", "I -2")
   << SpacegroupSymbolInfo(8, "a1", "B m 1 1", "", "B -2x")
   << SpacegroupSymbolInfo(8, "a2", "C m 1 1", "", "C -2x")
   << SpacegroupSymbolInfo(8, "a3", "I m 1 1", "", "I -2x")

   << SpacegroupSymbolInfo(9, "", "C c", "", "C -2yc")
   << SpacegroupSymbolInfo(9, "", "A n", "", "A -2yac")
   << SpacegroupSymbolInfo(9, "", "I a", "", "I -2ya")
   << SpacegroupSymbolInfo(9, "b1", "C 1 c 1", "", "C -2yc")
   << SpacegroupSymbolInfo(9, "b2", "A 1 n 1", "", "A -2yac")
   << SpacegroupSymbolInfo(9, "b3", "I 1 a 1", "", "I -2ya")
   << SpacegroupSymbolInfo(9, "-b1", "A 1 a 1", "", "A -2ya")
   << SpacegroupSymbolInfo(9, "-b2", "C 1 n 1", "", "C -2ybc")
   << SpacegroupSymbolInfo(9, "-b3", "I 1 c 1", "", "I -2yc")
   << SpacegroupSymbolInfo(9, "c1", "A 1 1 a", "", "A -2a")
   << SpacegroupSymbolInfo(9, "c2", "B 1 1 n", "", "B -2bc")
   << SpacegroupSymbolInfo(9, "c3", "I 1 1 b", "", "I -2b")
   << SpacegroupSymbolInfo(9, "-c1", "B 1 1 b", "", "B -2b")
   << SpacegroupSymbolInfo(9, "-c2", "A 1 1 n", "", "A -2ac")
   << SpacegroupSymbolInfo(9, "-c3", "I 1 1 a", "", "I -2a")
   << SpacegroupSymbolInfo(9, "a1", "B b 1 1", "", "B -2xb")
   << SpacegroupSymbolInfo(9, "a2", "C n 1 1", "", "C -2xbc")
   << SpacegroupSymbolInfo(9, "a3", "I c 1 1", "", "I -2xc")
   << SpacegroupSymbolInfo(9, "-a1", "C c 1 1", "", "C -2xc")
   << SpacegroupSymbolInfo(9, "-a2", "B n 1 1", "", "B -2xbc")
   << SpacegroupSymbolInfo(9, "-a3", "I b 1 1", "", "I -2xb")

   << SpacegroupSymbolInfo(10, "", "P 2/m", "", "-P 2y")
   << SpacegroupSymbolInfo(10, "b", "P 1 2/m 1", "", "-P 2y")
   << SpacegroupSymbolInfo(10, "c", "P 1 1 2/m", "", "-P 2")
   << SpacegroupSymbolInfo(10, "a", "P 2/m 1 1", "", "-P 2x")

   << SpacegroupSymbolInfo(11, "", "P 21/m", "", "-P 2yb")
   << SpacegroupSymbolInfo(11, "b", "P 1 21/m 1", "", "-P 2yb")
   << SpacegroupSymbolInfo(11, "c", "P 1 1 21/m", "", "-P 2c")
   << SpacegroupSymbolInfo(11, "a", "P 21/m 1 1", "", "-P 2xa")

   << SpacegroupSymbolInfo(12, "", "C 2/m", "", "-C 2y")
   << SpacegroupSymbolInfo(12, "", "A 2/m", "", "-A 2y")
   << SpacegroupSymbolInfo(12, "", "I 2/m", "", "-I 2y")
   << SpacegroupSymbolInfo(12, "b1", "C 1 2/m 1", "", "-C 2y")
   << SpacegroupSymbolInfo(12, "b2", "A 1 2/m 1", "", "-A 2y")
   << SpacegroupSymbolInfo(12, "b3", "I 1 2/m 1", "", "-I 2y")
   << SpacegroupSymbolInfo(12, "c1", "A 1 1 2/m", "", "-A 2")
   << SpacegroupSymbolInfo(12, "c2", "B 1 1 2/m", "", "-B 2")
   << SpacegroupSymbolInfo(12, "c3", "I 1 1 2/m", "", "-I 2")
   << SpacegroupSymbolInfo(12, "a1", "B 2/m 1 1", "", "-B 2x")
   << SpacegroupSymbolInfo(12, "a2", "C 2/m 1 1", "", "-C 2x")
   << SpacegroupSymbolInfo(12, "a3", "I 2/m 1 1", "", "-I 2x")

   << SpacegroupSymbolInfo(13, "", "P 2/c", "", "-P 2yc")
   << SpacegroupSymbolInfo(13, "", "P 2/n", "", "-P 2yac")
   << SpacegroupSymbolInfo(13, "", "P 2/a", "", "-P 2ya")
   << SpacegroupSymbolInfo(13, "b1", "P 1 2/c 1", "", "-P 2yc")
   << SpacegroupSymbolInfo(13, "b2", "P 1 2/n 1", "", "-P 2yac")
   << SpacegroupSymbolInfo(13, "b3", "P 1 2/a 1", "", "-P 2ya")
   << SpacegroupSymbolInfo(13, "c1", "P 1 1 2/a", "", "-P 2a")
   << SpacegroupSymbolInfo(13, "c2", "P 1 1 2/n", "", "-P 2ab")
   << SpacegroupSymbolInfo(13, "c3", "P 1 1 2/b", "", "-P 2b")
   << SpacegroupSymbolInfo(13, "a1", "P 2/b 1 1", "", "-P 2xb")
   << SpacegroupSymbolInfo(13, "a2", "P 2/n 1 1", "", "-P 2xbc")
   << SpacegroupSymbolInfo(13, "a3", "P 2/c 1 1", "", "-P 2xc")

   << SpacegroupSymbolInfo(14, "", "P 21/c", "", "-P 2ybc")
   << SpacegroupSymbolInfo(14, "", "P 21/n", "", "-P 2yn")
   << SpacegroupSymbolInfo(14, "", "P 21/a", "", "-P 2yab")
   << SpacegroupSymbolInfo(14, "b1", "P 1 21/c 1", "", "-P 2ybc")
   << SpacegroupSymbolInfo(14, "b2", "P 1 21/n 1", "", "-P 2yn")
   << SpacegroupSymbolInfo(14, "b3", "P 1 21/a 1", "", "-P 2yab")
   << SpacegroupSymbolInfo(14, "c1", "P 1 1 21/a", "", "-P 2ac")
   << SpacegroupSymbolInfo(14, "c2", "P 1 1 21/n", "", "-P 2n")
   << SpacegroupSymbolInfo(14, "c3", "P 1 1 21/b", "", "-P 2bc")
   << SpacegroupSymbolInfo(14, "a1", "P 21/b 1 1", "", "-P 2xab")
   << SpacegroupSymbolInfo(14, "a2", "P 21/n 1 1", "", "-P 2xn")
   << SpacegroupSymbolInfo(14, "a3", "P 21/c 1 1", "", "-P 2xac")

   << SpacegroupSymbolInfo(15, "", "C 2/c", "", "-C 2yc")
   << SpacegroupSymbolInfo(15, "", "A 2/n", "", "-A 2yac")
   << SpacegroupSymbolInfo(15, "", "I 2/a", "", "-I 2ya")
   << SpacegroupSymbolInfo(15, "b1", "C 1 2/c 1", "", "-C 2yc")
   << SpacegroupSymbolInfo(15, "b2", "A 1 2/n 1", "", "-A 2yac")
   << SpacegroupSymbolInfo(15, "b3", "I 1 2/a 1", "", "-I 2ya")
   << SpacegroupSymbolInfo(15, "-b1", "A 1 2/a 1", "", "-A 2ya")
   << SpacegroupSymbolInfo(15, "-b2", "C 1 2/n 1", "", "-C 2ybc")
   << SpacegroupSymbolInfo(15, "-b3", "I 1 2/c 1", "", "-I 2yc")
   << SpacegroupSymbolInfo(15, "c1", "A 1 1 2/a", "", "-A 2a")
   << SpacegroupSymbolInfo(15, "c2", "B 1 1 2/n", "", "-B 2bc")
   << SpacegroupSymbolInfo(15, "c3", "I 1 1 2/b", "", "-I 2b")
   << SpacegroupSymbolInfo(15, "-c1", "B 1 1 2/b", "", "-B 2b")
   << SpacegroupSymbolInfo(15, "-c2", "A 1 1 2/n", "", "-A 2ac")
   << SpacegroupSymbolInfo(15, "-c3", "I 1 1 2/a", "", "-I 2a")
   << SpacegroupSymbolInfo(15, "a1", "B 2/b 1 1", "", "-B 2xb")
   << SpacegroupSymbolInfo(15, "a2", "C 2/n 1 1", "", "-C 2xbc")
   << SpacegroupSymbolInfo(15, "a3", "I 2/c 1 1", "", "-I 2xc")
   << SpacegroupSymbolInfo(15, "-a1", "C 2/c 1 1", "", "-C 2xc")
   << SpacegroupSymbolInfo(15, "-a2", "B 2/n 1 1", "", "-B 2xbc")
   << SpacegroupSymbolInfo(15, "-a3", "I 2/b 1 1", "", "-I 2xb")

   << SpacegroupSymbolInfo(16, "", "P 2 2 2", "", "P 2 2")

   << SpacegroupSymbolInfo(17, "", "P 2 2 21", "", "P 2c 2")
   << SpacegroupSymbolInfo(17, "cab", "P 21 2 2", "", "P 2a 2a")
   << SpacegroupSymbolInfo(17, "bca", "P 2 21 2", "", "P 2 2b")

   << SpacegroupSymbolInfo(18, "", "P 21 21 2", "", "P 2 2ab")
   << SpacegroupSymbolInfo(18, "cab", "P 2 21 21", "", "P 2bc 2")
   << SpacegroupSymbolInfo(18, "bca", "P 21 2 21", "", "P 2ac 2ac")

   << SpacegroupSymbolInfo(19, "", "P 21 21 21", "", "P 2ac 2ab")

   << SpacegroupSymbolInfo(20, "", "C 2 2 21", "", "C 2c 2")
   << SpacegroupSymbolInfo(20, "cab", "A 21 2 2", "", "A 2a 2a")
   << SpacegroupSymbolInfo(20, "bca", "B 2 21 2", "", "B 2 2b")

   << SpacegroupSymbolInfo(21, "", "C 2 2 2", "", "C 2 2")
   << SpacegroupSymbolInfo(21, "cab", "A 2 2 2", "", "A 2 2")
   << SpacegroupSymbolInfo(21, "bca", "B 2 2 2", "", "B 2 2")

   << SpacegroupSymbolInfo(22, "", "F 2 2 2", "", "F 2 2")

   << SpacegroupSymbolInfo(23, "", "I 2 2 2", "", "I 2 2")

   << SpacegroupSymbolInfo(24, "", "I 21 21 21", "", "I 2b 2c")

   << SpacegroupSymbolInfo(25, "", "P m m 2", "", "P 2 -2")
   << SpacegroupSymbolInfo(25, "cab", "P 2 m m", "", "P -2 2")
   << SpacegroupSymbolInfo(25, "bca", "P m 2 m", "", "P -2 -2")

   << SpacegroupSymbolInfo(26, "", "P m c 21", "", "P 2c -2")
   << SpacegroupSymbolInfo(26, "ba-c", "P c m 21", "", "P 2c -2c")
   << SpacegroupSymbolInfo(26, "cab", "P 21 m a", "", "P -2a 2a")
   << SpacegroupSymbolInfo(26, "-cba", "P 21 a m", "", "P -2 2a")
   << SpacegroupSymbolInfo(26, "bca", "P b 21 m", "", "P -2 -2b")
   << SpacegroupSymbolInfo(26, "a-cb", "P m 21 b", "", "P -2b -2")

   << SpacegroupSymbolInfo(27, "", "P c c 2", "", "P 2 -2c")
   << SpacegroupSymbolInfo(27, "cab", "P 2 a a", "", "P -2a 2")
   << SpacegroupSymbolInfo(27, "bca", "P b 2 b", "", "P -2b -2b")

   << SpacegroupSymbolInfo(28, "", "P m a 2", "", "P 2 -2a")
   << SpacegroupSymbolInfo(28, "ba-c", "P b m 2", "", "P 2 -2b")
   << SpacegroupSymbolInfo(28, "cab", "P 2 m b", "", "P -2b 2")
   << SpacegroupSymbolInfo(28, "-cba", "P 2 c m", "", "P -2c 2")
   << SpacegroupSymbolInfo(28, "bca", "P c 2 m", "", "P -2c -2c")
   << SpacegroupSymbolInfo(28, "a-cb", "P m 2 a", "", "P -2a -2a")

   << SpacegroupSymbolInfo(29, "", "P c a 21", "", "P 2c -2ac")
   << SpacegroupSymbolInfo(29, "ba-c", "P b c 21", "", "P 2c -2b")
   << SpacegroupSymbolInfo(29, "cab", "P 21 a b", "", "P -2b 2a")
   << SpacegroupSymbolInfo(29, "-cba", "P 21 c a", "", "P -2ac 2a")
   << SpacegroupSymbolInfo(29, "bca", "P c 21 b", "", "P -2bc -2c")
   << SpacegroupSymbolInfo(29, "a-cb", "P b 21 a", "", "P -2a -2ab")

   << SpacegroupSymbolInfo(30, "", "P n c 2", "", "P 2 -2bc")
   << SpacegroupSymbolInfo(30, "ba-c", "P c n 2", "", "P 2 -2ac")
   << SpacegroupSymbolInfo(30, "cab", "P 2 n a", "", "P -2ac 2")
   << SpacegroupSymbolInfo(30, "-cba", "P 2 a n", "", "P -2ab 2")
   << SpacegroupSymbolInfo(30, "bca", "P b 2 n", "", "P -2ab -2ab")
   << SpacegroupSymbolInfo(30, "a-cb", "P n 2 b", "", "P -2bc -2bc")

   << SpacegroupSymbolInfo(31, "", "P m n 21", "", "P 2ac -2")
   << SpacegroupSymbolInfo(31, "ba-c", "P n m 21", "", "P 2bc -2bc")
   << SpacegroupSymbolInfo(31, "cab", "P 21 m n", "", "P -2ab 2ab")
   << SpacegroupSymbolInfo(31, "-cba", "P 21 n m", "", "P -2 2ac")
   << SpacegroupSymbolInfo(31, "bca", "P n 21 m", "", "P -2 -2bc")
   << SpacegroupSymbolInfo(31, "a-cb", "P m 21 n", "", "P -2ab -2")

   << SpacegroupSymbolInfo(32, "", "P b a 2", "", "P 2 -2ab")
   << SpacegroupSymbolInfo(32, "cab", "P 2 c b", "", "P -2bc 2")
   << SpacegroupSymbolInfo(32, "bca", "P c 2 a", "", "P -2ac -2ac")

   << SpacegroupSymbolInfo(33, "", "P n a 21", "", "P 2c -2n")
   << SpacegroupSymbolInfo(33, "ba-c", "P b n 21", "", "P 2c -2ab")
   << SpacegroupSymbolInfo(33, "cab", "P 21 n b", "", "P -2bc 2a")
   << SpacegroupSymbolInfo(33, "-cba", "P 21 c n", "", "P -2n 2a")
   << SpacegroupSymbolInfo(33, "bca", "P c 21 n", "", "P -2n -2ac")
   << SpacegroupSymbolInfo(33, "a-cb", "P n 21 a", "", "P -2ac -2n")

   << SpacegroupSymbolInfo(34, "", "P n n 2", "", "P 2 -2n")
   << SpacegroupSymbolInfo(34, "cab", "P 2 n n", "", "P -2n 2")
   << SpacegroupSymbolInfo(34, "bca", "P n 2 n", "", "P -2n -2n")

   << SpacegroupSymbolInfo(35, "", "C m m 2", "", "C 2 -2")
   << SpacegroupSymbolInfo(35, "cab", "A 2 m m", "", "A -2 2")
   << SpacegroupSymbolInfo(35, "bca", "B m 2 m", "", "B -2 -2")

   << SpacegroupSymbolInfo(36, "", "C m c 21", "", "C 2c -2")
   << SpacegroupSymbolInfo(36, "ba-c", "C c m 21", "", "C 2c -2c")
   << SpacegroupSymbolInfo(36, "cab", "A 21 m a", "", "A -2a 2a")
   << SpacegroupSymbolInfo(36, "-cba", "A 21 a m", "", "A -2 2a")
   << SpacegroupSymbolInfo(36, "bca", "B b 21 m", "", "B -2 -2b")
   << SpacegroupSymbolInfo(36, "a-cb", "B m 21 b", "", "B -2b -2")

   << SpacegroupSymbolInfo(37, "", "C c c 2", "", "C 2 -2c")
   << SpacegroupSymbolInfo(37, "cab", "A 2 a a", "", "A -2a 2")
   << SpacegroupSymbolInfo(37, "bca", "B b 2 b", "", "B -2b -2b")

   << SpacegroupSymbolInfo(38, "", "A m m 2", "", "A 2 -2")
   << SpacegroupSymbolInfo(38, "ba-c", "B m m 2", "", "B 2 -2")
   << SpacegroupSymbolInfo(38, "cab", "B 2 m m", "", "B -2 2")
   << SpacegroupSymbolInfo(38, "-cba", "C 2 m m", "", "C -2 2")
   << SpacegroupSymbolInfo(38, "bca", "C m 2 m", "", "C -2 -2")
   << SpacegroupSymbolInfo(38, "a-cb", "A m 2 m", "", "A -2 -2")

   << SpacegroupSymbolInfo(39, "", "A b m 2", "", "A 2 -2c")
   << SpacegroupSymbolInfo(39, "ba-c", "B m a 2", "", "B 2 -2c")
   << SpacegroupSymbolInfo(39, "cab", "B 2 c m", "", "B -2c 2")
   << SpacegroupSymbolInfo(39, "-cba", "C 2 m b", "", "C -2b 2")
   << SpacegroupSymbolInfo(39, "bca", "C m 2 a", "", "C -2b -2b")
   << SpacegroupSymbolInfo(39, "a-cb", "A c 2 m", "", "A -2c -2c")

   << SpacegroupSymbolInfo(40, "", "A m a 2", "", "A 2 -2a")
   << SpacegroupSymbolInfo(40, "ba-c", "B b m 2", "", "B 2 -2b")
   << SpacegroupSymbolInfo(40, "cab", "B 2 m b", "", "B -2b 2")
   << SpacegroupSymbolInfo(40, "-cba", "C 2 c m", "", "C -2c 2")
   << SpacegroupSymbolInfo(40, "bca", "C c 2 m", "", "C -2c -2c")
   << SpacegroupSymbolInfo(40, "a-cb", "A m 2 a", "", "A -2a -2a")

   << SpacegroupSymbolInfo(41, "", "A b a 2", "", "A 2 -2ac")
   << SpacegroupSymbolInfo(41, "ba-c", "B b a 2", "", "B 2 -2bc")
   << SpacegroupSymbolInfo(41, "cab", "B 2 c b", "", "B -2bc 2")
   << SpacegroupSymbolInfo(41, "-cba", "C 2 c b", "", "C -2bc 2")
   << SpacegroupSymbolInfo(41, "bca", "C c 2 a", "", "C -2bc -2bc")
   << SpacegroupSymbolInfo(41, "a-cb", "A c 2 a", "", "A -2ac -2ac")

   << SpacegroupSymbolInfo(42, "", "F m m 2", "", "F 2 -2")
   << SpacegroupSymbolInfo(42, "cab", "F 2 m m", "", "F -2 2")
   << SpacegroupSymbolInfo(42, "bca", "F m 2 m", "", "F -2 -2")

   << SpacegroupSymbolInfo(43, "", "F d d 2", "", "F 2 -2d")
   << SpacegroupSymbolInfo(43, "cab", "F 2 d d", "", "F -2d 2")
   << SpacegroupSymbolInfo(43, "bca", "F d 2 d", "", "F -2d -2d")

   << SpacegroupSymbolInfo(44, "", "I m m 2", "", "I 2 -2")
   << SpacegroupSymbolInfo(44, "cab", "I 2 m m", "", "I -2 2")
   << SpacegroupSymbolInfo(44, "bca", "I m 2 m", "", "I -2 -2")

   << SpacegroupSymbolInfo(45, "", "I b a 2", "", "I 2 -2c")
   << SpacegroupSymbolInfo(45, "cab", "I 2 c b", "", "I -2a 2")
   << SpacegroupSymbolInfo(45, "bca", "I c 2 a", "", "I -2b -2b")

   << SpacegroupSymbolInfo(46, "", "I m a 2", "", "I 2 -2a")
   << SpacegroupSymbolInfo(46, "ba-c", "I b m 2", "", "I 2 -2b")
   << SpacegroupSymbolInfo(46, "cab", "I 2 m b", "", "I -2b 2")
   << SpacegroupSymbolInfo(46, "-cba", "I 2 c m", "", "I -2c 2")
   << SpacegroupSymbolInfo(46, "bca", "I c 2 m", "", "I -2c -2c")
   << SpacegroupSymbolInfo(46, "a-cb", "I m 2 a", "", "I -2a -2a")

   << SpacegroupSymbolInfo(47, "", "P m m m", "", "-P 2 2")

   << SpacegroupSymbolInfo(48, "1", "P n n n", "1", "P 2 2 -1n")
   << SpacegroupSymbolInfo(48, "2", "P n n n", "2", "-P 2ab 2bc")

   << SpacegroupSymbolInfo(49, "", "P c c m", "", "-P 2 2c")
   << SpacegroupSymbolInfo(49, "cab", "P m a a", "", "-P 2a 2")
   << SpacegroupSymbolInfo(49, "bca", "P b m b", "", "-P 2b 2b")

   << SpacegroupSymbolInfo(50, "1", "P b a n", "1", "P 2 2 -1ab")
   << SpacegroupSymbolInfo(50, "2", "P b a n", "2", "-P 2ab 2b")
   << SpacegroupSymbolInfo(50, "1cab", "P n c b", "1", "P 2 2 -1bc")
   << SpacegroupSymbolInfo(50, "2cab", "P n c b", "2", "-P 2b 2bc")
   << SpacegroupSymbolInfo(50, "1bca", "P c n a", "1", "P 2 2 -1ac")
   << SpacegroupSymbolInfo(50, "2bca", "P c n a", "2", "-P 2a 2c")

   << SpacegroupSymbolInfo(51, "", "P m m a", "", "-P 2a 2a")
   << SpacegroupSymbolInfo(51, "ba-c", "P m m b", "", "-P 2b 2")
   << SpacegroupSymbolInfo(51, "cab", "P b m m", "", "-P 2 2b")
   << SpacegroupSymbolInfo(51, "-cba", "P c m m", "", "-P 2c 2c")
   << SpacegroupSymbolInfo(51, "bca", "P m c m", "", "-P 2c 2")
   << SpacegroupSymbolInfo(51, "a-cb", "P m a m", "", "-P 2 2a")

   << SpacegroupSymbolInfo(52, "", "P n n a", "", "-P 2a 2bc")
   << SpacegroupSymbolInfo(52, "ba-c", "P n n b", "", "-P 2b 2n")
   << SpacegroupSymbolInfo(52, "cab", "P b n n", "", "-P 2n 2b")
   << SpacegroupSymbolInfo(52, "-cba", "P c n n", "", "-P 2ab 2c")
   << SpacegroupSymbolInfo(52, "bca", "P n c n", "", "-P 2ab 2n")
   << SpacegroupSymbolInfo(52, "a-cb", "P n a n", "", "-P 2n 2bc")

   << SpacegroupSymbolInfo(53, "", "P m n a", "", "-P 2ac 2")
   << SpacegroupSymbolInfo(53, "ba-c", "P n m b", "", "-P 2bc 2bc")
   << SpacegroupSymbolInfo(53, "cab", "P b m n", "", "-P 2ab 2ab")
   << SpacegroupSymbolInfo(53, "-cba", "P c n m", "", "-P 2 2ac")
   << SpacegroupSymbolInfo(53, "bca", "P n c m", "", "-P 2 2bc")
   << SpacegroupSymbolInfo(53, "a-cb", "P m a n", "", "-P 2ab 2")

   << SpacegroupSymbolInfo(54, "", "P c c a", "", "-P 2a 2ac")
   << SpacegroupSymbolInfo(54, "ba-c", "P c c b", "", "-P 2b 2c")
   << SpacegroupSymbolInfo(54, "cab", "P b a a", "", "-P 2a 2b")
   << SpacegroupSymbolInfo(54, "-cba", "P c a a", "", "-P 2ac 2c")
   << SpacegroupSymbolInfo(54, "bca", "P b c b", "", "-P 2bc 2b")
   << SpacegroupSymbolInfo(54, "a-cb", "P b a b", "", "-P 2b 2ab")

   << SpacegroupSymbolInfo(55, "", "P b a m", "", "-P 2 2ab")
   << SpacegroupSymbolInfo(55, "cab", "P m c b", "", "-P 2bc 2")
   << SpacegroupSymbolInfo(55, "bca", "P c m a", "", "-P 2ac 2ac")
   << SpacegroupSymbolInfo(56, "", "P c c n", "", "-P 2ab 2ac")
   << SpacegroupSymbolInfo(56, "cab", "P n a a", "", "-P 2ac 2bc")
   << SpacegroupSymbolInfo(56, "bca", "P b n b", "", "-P 2bc 2ab")

   << SpacegroupSymbolInfo(57, "", "P b c m", "", "-P 2c 2b")
   << SpacegroupSymbolInfo(57, "ba-c", "P c a m", "", "-P 2c 2ac")
   << SpacegroupSymbolInfo(57, "cab", "P m c a", "", "-P 2ac 2a")
   << SpacegroupSymbolInfo(57, "-cba", "P m a b", "", "-P 2b 2a")
   << SpacegroupSymbolInfo(57, "bca", "P b m a", "", "-P 2a 2ab")
   << SpacegroupSymbolInfo(57, "a-cb", "P c m b", "", "-P 2bc 2c")

   << SpacegroupSymbolInfo(58, "", "P n n m", "", "-P 2 2n")
   << SpacegroupSymbolInfo(58, "cab", "P m n n", "", "-P 2n 2")
   << SpacegroupSymbolInfo(58, "bca", "P n m n", "", "-P 2n 2n")

   << SpacegroupSymbolInfo(59, "1", "P m m n", "1", "P 2 2ab -1ab")
   << SpacegroupSymbolInfo(59, "2", "P m m n", "2", "-P 2ab 2a")
   << SpacegroupSymbolInfo(59, "1cab", "P n m m", "1", "P 2bc 2 -1bc")
   << SpacegroupSymbolInfo(59, "2cab", "P n m m", "2", "-P 2c 2bc")
   << SpacegroupSymbolInfo(59, "1bca", "P m n m", "1", "P 2ac 2ac -1ac")
   << SpacegroupSymbolInfo(59, "2bca", "P m n m", "2", "-P 2c 2a")

   << SpacegroupSymbolInfo(60, "", "P b c n", "", "-P 2n 2ab")
   << SpacegroupSymbolInfo(60, "ba-c", "P c a n", "", "-P 2n 2c")
   << SpacegroupSymbolInfo(60, "cab", "P n c a", "", "-P 2a 2n")
   << SpacegroupSymbolInfo(60, "-cba", "P n a b", "", "-P 2bc 2n")
   << SpacegroupSymbolInfo(60, "bca", "P b n a", "", "-P 2ac 2b")
   << SpacegroupSymbolInfo(60, "a-cb", "P c n b", "", "-P 2b 2ac")

   << SpacegroupSymbolInfo(61, "", "P b c a", "", "-P 2ac 2ab")
   << SpacegroupSymbolInfo(61, "ba-c", "P c a b", "", "-P 2bc 2ac")

   << SpacegroupSymbolInfo(62, "", "P n m a", "", "-P 2ac 2n")
   << SpacegroupSymbolInfo(62, "ba-c", "P m n b", "", "-P 2bc 2a")
   << SpacegroupSymbolInfo(62, "cab", "P b n m", "", "-P 2c 2ab")
   << SpacegroupSymbolInfo(62, "-cba", "P c m n", "", "-P 2n 2ac")
   << SpacegroupSymbolInfo(62, "bca", "P m c n", "", "-P 2n 2a")
   << SpacegroupSymbolInfo(62, "a-cb", "P n a m", "", "-P 2c 2n")

   << SpacegroupSymbolInfo(63, "", "C m c m", "", "-C 2c 2")
   << SpacegroupSymbolInfo(63, "ba-c", "C c m m", "", "-C 2c 2c")
   << SpacegroupSymbolInfo(63, "cab", "A m m a", "", "-A 2a 2a")
   << SpacegroupSymbolInfo(63, "-cba", "A m a m", "", "-A 2 2a")
   << SpacegroupSymbolInfo(63, "bca", "B b m m", "", "-B 2 2b")
   << SpacegroupSymbolInfo(63, "a-cb", "B m m b", "", "-B 2b 2")

   << SpacegroupSymbolInfo(64, "", "C m c a", "", "-C 2bc 2")
   << SpacegroupSymbolInfo(64, "ba-c", "C c m b", "", "-C 2bc 2bc")
   << SpacegroupSymbolInfo(64, "cab", "A b m a", "", "-A 2ac 2ac")
   << SpacegroupSymbolInfo(64, "-cba", "A c a m", "", "-A 2 2ac")
   << SpacegroupSymbolInfo(64, "bca", "B b c m", "", "-B 2 2bc")
   << SpacegroupSymbolInfo(64, "a-cb", "B m a b", "", "-B 2bc 2")

   << SpacegroupSymbolInfo(65, "", "C m m m", "", "-C 2 2")
   << SpacegroupSymbolInfo(65, "cab", "A m m m", "", "-A 2 2")
   << SpacegroupSymbolInfo(65, "bca", "B m m m", "", "-B 2 2")

   << SpacegroupSymbolInfo(66, "", "C c c m", "", "-C 2 2c")
   << SpacegroupSymbolInfo(66, "cab", "A m a a", "", "-A 2a 2")
   << SpacegroupSymbolInfo(66, "bca", "B b m b", "", "-B 2b 2b")

   << SpacegroupSymbolInfo(67, "", "C m m a", "", "-C 2b 2")
   << SpacegroupSymbolInfo(67, "ba-c", "C m m b", "", "-C 2b 2b")
   << SpacegroupSymbolInfo(67, "cab", "A b m m", "", "-A 2c 2c")
   << SpacegroupSymbolInfo(67, "-cba", "A c m m", "", "-A 2 2c")
   << SpacegroupSymbolInfo(67, "bca", "B m c m", "", "-B 2 2c")
   << SpacegroupSymbolInfo(67, "a-cb", "B m a m", "", "-B 2c 2")

   << SpacegroupSymbolInfo(68, "1", "C c c a", "1", "C 2 2 -1bc")
   << SpacegroupSymbolInfo(68, "2", "C c c a", "2", "-C 2b 2bc")
   << SpacegroupSymbolInfo(68, "1ba-c", "C c c b", "1", "C 2 2 -1bc")
   << SpacegroupSymbolInfo(68, "2ba-c", "C c c b", "2", "-C 2b 2c")
   << SpacegroupSymbolInfo(68, "1cab", "A b a a", "1", "A 2 2 -1ac")
   << SpacegroupSymbolInfo(68, "2cab", "A b a a", "2", "-A 2a 2c")
   << SpacegroupSymbolInfo(68, "1-cba", "A c a a", "1", "A 2 2 -1ac")
   << SpacegroupSymbolInfo(68, "2-cba", "A c a a", "2", "-A 2ac 2c")
   << SpacegroupSymbolInfo(68, "1bca", "B b c b", "1", "B 2 2 -1bc")
   << SpacegroupSymbolInfo(68, "2bca", "B b c b", "2", "-B 2bc 2b")
   << SpacegroupSymbolInfo(68, "1a-cb", "B b a b", "1", "B 2 2 -1bc")
   << SpacegroupSymbolInfo(68, "2a-cb", "B b a b", "2", "-B 2b 2bc")

   << SpacegroupSymbolInfo(69, "", "F m m m", "", "-F 2 2")

   << SpacegroupSymbolInfo(70, "1", "F d d d", "1", "F 2 2 -1d")
   << SpacegroupSymbolInfo(70, "2", "F d d d", "2", "-F 2uv 2vw")

   << SpacegroupSymbolInfo(71, "", "I m m m", "", "-I 2 2")

   << SpacegroupSymbolInfo(72, "", "I b a m", "", "-I 2 2c")
   << SpacegroupSymbolInfo(72, "cab", "I m c b", "", "-I 2a 2")
   << SpacegroupSymbolInfo(72, "bca", "I c m a", "", "-I 2b 2b")

   << SpacegroupSymbolInfo(73, "", "I b c a", "", "-I 2b 2c")
   << SpacegroupSymbolInfo(73, "ba-c", "I c a b", "", "-I 2a 2b")

   << SpacegroupSymbolInfo(74, "", "I m m a", "", "-I 2b 2")
   << SpacegroupSymbolInfo(74, "ba-c", "I m m b", "", "-I 2a 2a")
   << SpacegroupSymbolInfo(74, "cab", "I b m m", "", "-I 2c 2c")
   << SpacegroupSymbolInfo(74, "-cba", "I c m m", "", "-I 2 2b")
   << SpacegroupSymbolInfo(74, "bca", "I m c m", "", "-I 2 2a")
   << SpacegroupSymbolInfo(74, "a-cb", "I m a m", "", "-I 2c 2")

   << SpacegroupSymbolInfo(75, "", "P 4", "", "P 4")

   << SpacegroupSymbolInfo(76, "", "P 41", "", "P 4w")

   << SpacegroupSymbolInfo(77, "", "P 42", "", "P 4c")

   << SpacegroupSymbolInfo(78, "", "P 43", "", "P 4cw")

   << SpacegroupSymbolInfo(79, "", "I 4", "", "I 4")

   << SpacegroupSymbolInfo(80, "", "I 41", "", "I 4bw")

   << SpacegroupSymbolInfo(81, "", "P -4", "", "P -4")

   << SpacegroupSymbolInfo(82, "", "I -4", "", "I -4")

   << SpacegroupSymbolInfo(83, "", "P 4/m", "", "-P 4")

   << SpacegroupSymbolInfo(84, "", "P 42/m", "", "-P 4c")

   << SpacegroupSymbolInfo(85, "1", "P 4/n", "1", "P 4ab -1ab")
   << SpacegroupSymbolInfo(85, "2", "P 4/n", "2", "-P 4a")

   << SpacegroupSymbolInfo(86, "1", "P 42/n", "1", "P 4n -1n")
   << SpacegroupSymbolInfo(86, "2", "P 42/n", "2", "-P 4bc")

   << SpacegroupSymbolInfo(87, "", "I 4/m", "", "-I 4")

   << SpacegroupSymbolInfo(88, "1", "I 41/a", "1", "I 4bw -1bw")
   << SpacegroupSymbolInfo(88, "2", "I 41/a", "2", "-I 4ad")

   << SpacegroupSymbolInfo(89, "", "P 4 2 2", "", "P 4 2")

   << SpacegroupSymbolInfo(90, "", "P 42 1 2", "", "P 4ab 2ab")

   << SpacegroupSymbolInfo(91, "", "P 41 2 2", "", "P 4w 2c")

   << SpacegroupSymbolInfo(92, "", "P 41 21 2", "", "P 4abw 2nw")

   << SpacegroupSymbolInfo(93, "", "P 42 2 2", "", "P 4c 2")

   << SpacegroupSymbolInfo(94, "", "P 42 21 2", "", "P 4n 2n")

   << SpacegroupSymbolInfo(95, "", "P 43 2 2", "", "P 4cw 2c")

   << SpacegroupSymbolInfo(96, "", "P 43 21 2", "", "P 4nw 2abw")

   << SpacegroupSymbolInfo(97, "", "I 4 2 2", "", "I 4 2")

   << SpacegroupSymbolInfo(98, "", "I 41 2 2", "", "I 4bw 2bw")

   << SpacegroupSymbolInfo(99, "", "P 4 m m", "", "P 4 -2")

   << SpacegroupSymbolInfo(100, "", "P 4 b m", "", "P 4 -2ab")

   << SpacegroupSymbolInfo(101, "", "P 42 c m", "", "P 4c -2c")

   << SpacegroupSymbolInfo(102, "", "P 42 n m", "", "P 4n -2n")

   << SpacegroupSymbolInfo(103, "", "P 4 c c", "", "P 4 -2c")

   << SpacegroupSymbolInfo(104, "", "P 4 n c", "", "P 4 -2n")

   << SpacegroupSymbolInfo(105, "", "P 42 m c", "", "P 4c -2")

   << SpacegroupSymbolInfo(106, "", "P 42 b c", "", "P 4c -2ab")

   << SpacegroupSymbolInfo(107, "", "I 4 m m", "", "I 4 -2")

   << SpacegroupSymbolInfo(108, "", "I 4 c m", "", "I 4 -2c")

   << SpacegroupSymbolInfo(109, "", "I 41 m d", "", "I 4bw -2")

   << SpacegroupSymbolInfo(110, "", "I 41 c d", "", "I 4bw -2c")

   << SpacegroupSymbolInfo(111, "", "P -4 2 m", "", "P -4 2")

   << SpacegroupSymbolInfo(112, "", "P -4 2 c", "", "P -4 2c")

   << SpacegroupSymbolInfo(113, "", "P -4 21 m", "", "P -4 2ab")

   << SpacegroupSymbolInfo(114, "", "P -4 21 c", "", "P -4 2n")

   << SpacegroupSymbolInfo(115, "", "P -4 m 2", "", "P -4 -2")

   << SpacegroupSymbolInfo(116, "", "P -4 c 2", "", "P -4 -2c")

   << SpacegroupSymbolInfo(117, "", "P -4 b 2", "", "P -4 -2ab")

   << SpacegroupSymbolInfo(118, "", "P -4 n 2", "", "P -4 -2n")

   << SpacegroupSymbolInfo(119, "", "I -4 m 2", "", "I -4 -2")

   << SpacegroupSymbolInfo(120, "", "I -4 c 2", "", "I -4 -2c")

   << SpacegroupSymbolInfo(121, "", "I -4 2 m", "", "I -4 2")

   << SpacegroupSymbolInfo(122, "", "I -4 2 d", "", "I -4 2bw")

   << SpacegroupSymbolInfo(123, "", "P 4/m m m", "", "-P 4 2")

   << SpacegroupSymbolInfo(124, "", "P 4/m c c", "", "-P 4 2c")

   << SpacegroupSymbolInfo(125, "1", "P 4/n b m", "1", "P 4 2 -1ab")
   << SpacegroupSymbolInfo(125, "2", "P 4/n b m", "2", "-P 4a 2b")

   << SpacegroupSymbolInfo(126, "1", "P 4/n n c", "1", "P 4 2 -1n")
   << SpacegroupSymbolInfo(126, "2", "P 4/n n c", "2", "-P 4a 2bc")

   << SpacegroupSymbolInfo(127, "", "P 4/m b m", "", "-P 4 2ab")

   << SpacegroupSymbolInfo(128, "", "P 4/m n c", "", "-P 4 2n")

   << SpacegroupSymbolInfo(129, "1", "P 4/n m m", "1", "P 4ab 2ab -1ab")
   << SpacegroupSymbolInfo(129, "2", "P 4/n m m", "2", "-P 4a 2a")

   << SpacegroupSymbolInfo(130, "1", "P 4/n c c", "1", "P 4ab 2n -1ab")
   << SpacegroupSymbolInfo(130, "2", "P 4/n c c", "2", "-P 4a 2ac")

   << SpacegroupSymbolInfo(131, "", "P 42/m m c", "", "-P 4c 2")

   << SpacegroupSymbolInfo(132, "", "P 42/m c m", "", "-P 4c 2c")

   << SpacegroupSymbolInfo(133, "1", "P 42/n b c", "1", "P 4n 2c -1n")
   << SpacegroupSymbolInfo(133, "2", "P 42/n b c", "2", "-P 4ac 2b")

   << SpacegroupSymbolInfo(134, "1", "P 42/n n m", "1", "P 4n 2 -1n")
   << SpacegroupSymbolInfo(134, "2", "P 42/n n m", "2", "-P 4ac 2bc")

   << SpacegroupSymbolInfo(135, "", "P 42/m b c", "", "-P 4c 2ab")

   << SpacegroupSymbolInfo(136, "", "P 42/m n m", "", "-P 4n 2n")

   << SpacegroupSymbolInfo(137, "1", "P 42/n m c", "1", "P 4n 2n -1n")
   << SpacegroupSymbolInfo(137, "2", "P 42/n m c", "2", "-P 4ac 2a")

   << SpacegroupSymbolInfo(138, "1", "P 42/n c m", "1", "P 4n 2ab -1n")
   << SpacegroupSymbolInfo(138, "2", "P 42/n c m", "2", "-P 4ac 2ac")

   << SpacegroupSymbolInfo(139, "", "I 4/m m m", "", "-I 4 2")
   << SpacegroupSymbolInfo(139, "", "F 4/m m m", "", "-F 4 2")

   << SpacegroupSymbolInfo(140, "", "I 4/m c m", "", "-I 4 2c")

   << SpacegroupSymbolInfo(141, "1", "I 41/a m d", "1", "I 4bw 2bw -1bw")
   << SpacegroupSymbolInfo(141, "2", "I 41/a m d", "2", "-I 4bd 2")

   << SpacegroupSymbolInfo(142, "1", "I 41/a c d", "1", "I 4bw 2aw -1bw")
   << SpacegroupSymbolInfo(142, "2", "I 41/a c d", "2", "-I 4bd 2c")

   << SpacegroupSymbolInfo(143, "", "P 3", "", "P 3")

   << SpacegroupSymbolInfo(144, "", "P 31", "", "P 31")

   << SpacegroupSymbolInfo(145, "", "P 32", "", "P 32")

   << SpacegroupSymbolInfo(146, "H", "R 3", "H", "R 3")
   << SpacegroupSymbolInfo(146, "R", "R 3", "R", "P 3*")

   << SpacegroupSymbolInfo(147, "", "P -3", "", "-P 3")

   << SpacegroupSymbolInfo(148, "H", "R -3", "H", "-R 3")
   << SpacegroupSymbolInfo(148, "R", "R -3", "R", "-P 3*")

   << SpacegroupSymbolInfo(149, "", "P 3 1 2", "", "P 3 2")

   << SpacegroupSymbolInfo(150, "", "P 3 2 1", "", "P 3 2\"")

   << SpacegroupSymbolInfo(151, "", "P 31 1 2", "", "P 31 2c (0 0 1)")

   << SpacegroupSymbolInfo(152, "", "P 31 2 1", "", "P 31 2\"")

   << SpacegroupSymbolInfo(153, "", "P 32 1 2", "", "P 32 2c (0 0 -1)")

   << SpacegroupSymbolInfo(154, "", "P 32 2 1", "", "P 32 2\"")

   << SpacegroupSymbolInfo(155, "H", "R 32", "H", "R 3 2\"")
   << SpacegroupSymbolInfo(155, "R", "R 32", "R", "P 3* 2")

   << SpacegroupSymbolInfo(156, "", "P 3 m 1", "", "P 3 -2\"")

   << SpacegroupSymbolInfo(157, "", "P 3 1 m", "", "P 3 -2")

   << SpacegroupSymbolInfo(158, "", "P 3 c 1", "", "P 3 -2\"c")

   << SpacegroupSymbolInfo(159, "", "P 3 1 c", "", "P 3 -2c")

   << SpacegroupSymbolInfo(160, "H", "R 3 m", "H", "R 3 -2\"")
   << SpacegroupSymbolInfo(160, "R", "R 3 m", "R", "P 3* -2")

   << SpacegroupSymbolInfo(161, "H", "R 3 c", "H", "R 3 -2\"c")
   << SpacegroupSymbolInfo(161, "R", "R 3 c", "R", "P 3* -2n")

   << SpacegroupSymbolInfo(162, "", "P -3 1 m", "", "-P 3 2")

   << SpacegroupSymbolInfo(163, "", "P -3 1 c", "", "-P 3 2c")

   << SpacegroupSymbolInfo(164, "", "P -3 m 1", "", "-P 3 2\"")

   << SpacegroupSymbolInfo(165, "", "P -3 c 1", "", "-P 3 2\"c")

   << SpacegroupSymbolInfo(166, "H", "R -3 m", "H", "-R 3 2\"")
   << SpacegroupSymbolInfo(166, "R", "R -3 m", "R", "-P 3* 2")

   << SpacegroupSymbolInfo(167, "H", "R -3 c", "H", "-R 3 2\"c")
   << SpacegroupSymbolInfo(167, "R", "R -3 c", "R", "-P 3* 2n")

   << SpacegroupSymbolInfo(168, "", "P 6", "", "P 6")

   << SpacegroupSymbolInfo(169, "", "P 61", "", "P 61")

   << SpacegroupSymbolInfo(170, "", "P 65", "", "P 65")

   << SpacegroupSymbolInfo(171, "", "P 62", "", "P 62")

   << SpacegroupSymbolInfo(172, "", "P 64", "", "P 64")

   << SpacegroupSymbolInfo(173, "", "P 63", "", "P 6c")

   << SpacegroupSymbolInfo(174, "", "P -6", "", "P -6")

   << SpacegroupSymbolInfo(175, "", "P 6/m", "", "-P 6")

   << SpacegroupSymbolInfo(176, "", "P 63/m", "", "-P 6c")

   << SpacegroupSymbolInfo(177, "", "P 6 2 2", "", "P 6 2")

   << SpacegroupSymbolInfo(178, "", "P 61 2 2", "", "P 61 2 (0 0 -1)")

   << SpacegroupSymbolInfo(179, "", "P 65 2 2", "", "P 65 2 (0 0 1)")

   << SpacegroupSymbolInfo(180, "", "P 62 2 2", "", "P 62 2c (0 0 1)")

   << SpacegroupSymbolInfo(181, "", "P 64 2 2", "", "P 64 2c (0 0 -1)")

   << SpacegroupSymbolInfo(182, "", "P 63 2 2", "", "P 6c 2c")

   << SpacegroupSymbolInfo(183, "", "P 6 m m", "", "P 6 -2")

   << SpacegroupSymbolInfo(184, "", "P 6 c c", "", "P 6 -2c")

   << SpacegroupSymbolInfo(185, "", "P 63 c m", "", "P 6c -2")

   << SpacegroupSymbolInfo(186, "", "P 63 m c", "", "P 6c -2c")

   << SpacegroupSymbolInfo(187, "", "P -6 m 2", "", "P -6 2")

   << SpacegroupSymbolInfo(188, "", "P -6 c 2", "", "P -6c 2")

   << SpacegroupSymbolInfo(189, "", "P -6 2 m", "", "P -6 -2")

   << SpacegroupSymbolInfo(190, "", "P -6 2 c", "", "P -6c -2c")

   << SpacegroupSymbolInfo(191, "", "P 6/m m m", "", "-P 6 2")

   << SpacegroupSymbolInfo(192, "", "P 6/m c c", "", "-P 6 2c")

   << SpacegroupSymbolInfo(193, "", "P 63/m c m", "", "-P 6c 2")

   << SpacegroupSymbolInfo(194, "", "P 63/m m c", "", "-P 6c 2c")

   << SpacegroupSymbolInfo(195, "", "P 2 3", "", "P 2 2 3")

   << SpacegroupSymbolInfo(196, "", "F 2 3", "", "F 2 2 3")

   << SpacegroupSymbolInfo(197, "", "I 2 3", "", "I 2 2 3")

   << SpacegroupSymbolInfo(198, "", "P 21 3", "", "P 2ac 2ab 3")

   << SpacegroupSymbolInfo(199, "", "I 21 3", "", "I 2b 2c 3")

   << SpacegroupSymbolInfo(200, "", "P m -3", "", "-P 2 2 3")
   << SpacegroupSymbolInfo(200, "", "P m 3", "", "-P 2 2 3")

   << SpacegroupSymbolInfo(201, "1", "P n -3", "1", "P 2 2 3 -1n")
   << SpacegroupSymbolInfo(201, "2", "P n -3", "2", "-P 2ab 2bc 3")
   << SpacegroupSymbolInfo(201, "1", "P n 3", "1", "P 2 2 3 -1n")

   << SpacegroupSymbolInfo(202, "", "F m -3", "", "-F 2 2 3")
   << SpacegroupSymbolInfo(202, "", "F m 3", "", "-F 2 2 3")

   << SpacegroupSymbolInfo(203, "1", "F d -3", "1", "F 2 2 3 -1d")
   << SpacegroupSymbolInfo(203, "2", "F d -3", "2", "-F 2uv 2vw 3")
   << SpacegroupSymbolInfo(203, "1", "F d 3", "1", "F 2 2 3 -1d")

   << SpacegroupSymbolInfo(204, "", "I m -3", "", "-I 2 2 3")
   << SpacegroupSymbolInfo(204, "", "I m 3", "", "-I 2 2 3")

   << SpacegroupSymbolInfo(205, "", "P a -3", "", "-P 2ac 2ab 3")
   << SpacegroupSymbolInfo(205, "", "P a 3", "", "-P 2ac 2ab 3")

   << SpacegroupSymbolInfo(206, "", "I a -3", "", "-I 2b 2c 3")
   << SpacegroupSymbolInfo(206, "", "I a 3", "", "-I 2b 2c 3")

   << SpacegroupSymbolInfo(207, "", "P 4 3 2", "", "P 4 2 3")

   << SpacegroupSymbolInfo(208, "", "P 42 3 2", "", "P 4n 2 3")

   << SpacegroupSymbolInfo(209, "", "F 4 3 2", "", "F 4 2 3")

   << SpacegroupSymbolInfo(210, "", "F 41 3 2", "", "F 4d 2 3")

   << SpacegroupSymbolInfo(211, "", "I 4 3 2", "", "I 4 2 3")

   << SpacegroupSymbolInfo(212, "", "P 43 3 2", "", "P 4acd 2ab 3")

   << SpacegroupSymbolInfo(213, "", "P 41 3 2", "", "P 4bd 2ab 3")

   << SpacegroupSymbolInfo(214, "", "I 41 3 2", "", "I 4bd 2c 3")

   << SpacegroupSymbolInfo(215, "", "P -4 3 m", "", "P -4 2 3")

   << SpacegroupSymbolInfo(216, "", "F -4 3 m", "", "F -4 2 3")

   << SpacegroupSymbolInfo(217, "", "I -4 3 m", "", "I -4 2 3")

   << SpacegroupSymbolInfo(218, "", "P -4 3 n", "", "P -4n 2 3")

   << SpacegroupSymbolInfo(219, "", "F -4 3 c", "", "F -4c 2 3")

   << SpacegroupSymbolInfo(220, "", "I -4 3 d", "", "I -4bd 2c 3")

   << SpacegroupSymbolInfo(221, "", "P m -3 m", "", "-P 4 2 3")
   << SpacegroupSymbolInfo(221, "", "P m 3 m", "", "-P 4 2 3")

   << SpacegroupSymbolInfo(222, "1", "P n -3 n", "1", "P 4 2 3 -1n")
   << SpacegroupSymbolInfo(222, "2", "P n -3 n", "2", "-P 4a 2bc 3")
   << SpacegroupSymbolInfo(222, "1", "P n 3 n", "1", "P 4 2 3 -1n")

   << SpacegroupSymbolInfo(223, "", "P m -3 n", "", "-P 4n 2 3")
   << SpacegroupSymbolInfo(223, "", "P m 3 n", "", "-P 4n 2 3")

   << SpacegroupSymbolInfo(224, "1", "P n -3 m", "1", "P 4n 2 3 -1n")
   << SpacegroupSymbolInfo(224, "2", "P n -3 m", "2", "-P 4bc 2bc 3")
   << SpacegroupSymbolInfo(224, "1", "P n 3 m", "1", "P 4n 2 3 -1n")

   << SpacegroupSymbolInfo(225, "", "F m -3 m", "", "-F 4 2 3")
   << SpacegroupSymbolInfo(225, "", "F m 3 m", "", "-F 4 2 3")

   << SpacegroupSymbolInfo(226, "", "F m -3 c", "", "-F 4c 2 3")
   << SpacegroupSymbolInfo(226, "", "F m 3 c", "", "-F 4c 2 3")

   << SpacegroupSymbolInfo(227, "1", "F d -3 m", "1", "F 4d 2 3 -1d")
   << SpacegroupSymbolInfo(227, "2", "F d -3 m", "2", "-F 4vw 2vw 3")
   << SpacegroupSymbolInfo(227, "1", "F d 3 m", "1", "F 4d 2 3 -1d")

   << SpacegroupSymbolInfo(228, "1", "F d -3 c", "1", "F 4d 2 3 -1cd")
   << SpacegroupSymbolInfo(228, "2", "F d -3 c", "2", "-F 4cvw 2vw 3")
   << SpacegroupSymbolInfo(228, "1", "F d 3 c", "1", "F 4d 2 3 -1cd")

   << SpacegroupSymbolInfo(229, "", "I m -3 m", "", "-I 4 2 3")
   << SpacegroupSymbolInfo(229, "", "I m 3 m", "", "-I 4 2 3")

   << SpacegroupSymbolInfo(230, "", "I a -3 d", "", "-I 4bd 2c 3")
   << SpacegroupSymbolInfo(230, "", "I a 3 d", "", "-I 4bd 2c 3");
}
