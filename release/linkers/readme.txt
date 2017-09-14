/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/



Hello, this boot folder consists of two parts:


arm7:
nds_arm7_ld.ld
nds_arm7_ld.specs
nds_arm7_ld_crt0.s

arm9:
nds_arm9_ld.ld
nds_arm9_ld.specs
nds_arm9_ld_crt0.s


1)
for ARM7 core you need to compile nds_arm7_ld_crt0.s, here is how to: 

copy nds_arm7_ld_crt0.s to arm7/source folder 

Make project, should generate link errors

go to arm7/build folder

move nds_arm7_ld_crt0.o to

(lib folder)
\generic_toolchain\6.2_2016q4\arm-none-eabi\lib

ONLY after copying nds_arm7_ld_crt0.o, you must delete nds_arm7_ld_crt0.s
from arm7/source folder

2)
for ARM9 core you need to compile nds_arm9_ld_crt0.s, here is how to: 

copy nds_arm9_ld_crt0.o to arm9/source folder 

Make project, should generate link errors

go to arm9/build folder

move nds_arm9_ld_crt0.o to

(lib folder)
\generic_toolchain\6.2_2016q4\arm-none-eabi\lib

ONLY after copying nds_arm9_ld_crt0.o, you must delete nds_arm9_ld_crt0.s
from arm9/source folder

3)

copy 
nds_arm7_ld.ld
nds_arm7_ld.specs
nds_arm9_ld.ld
nds_arm9_ld.specs

to
(lib folder)
\generic_toolchain\6.2_2016q4\arm-none-eabi\lib


Now Make your project again, it should build just fine.
This is something you compile once, unless nds_arm7_ld_crt0.s or nds_arm9_ld_crt0.s 
is updated in future toolchain releases. The latest crt0s (the files I wrote above),
will always be found inside the /linkers folder.