@echo off

set CommonCompilerFlags= -MT -GR- -EHa -Oi -W4 -WX -wd4201 -wd4100 -wd4189 -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_WIN32=1 -FC -Z7 -Fmwin32_handmade.map 
set CommonLinkerFlags= -opt:ref user32.lib Gdi32.lib Winmm.lib

REM 32-bit build
REM cl %CommonCompilerFlags% win32_handmade.cpp /link %CommonLinkerFlags

REM 64-bit build
cl %CommonCompilerFlags% win32_handmade.cpp /link %CommonLinkerFlags%



rem W4 for warning errer level 4, WX to treat warnings as errors
rem wd4201 for the nameless struct in testprogram.h
rem wd4100 for unrefrenced parameters
rem wd4189 for unused local variable
rem Zi produces debug information in a .pdb file
rem Z7 produces debug information in a .pdb file and supposedly plays nices with multi-processor builds than -Zi
rem Oi to always do any intrinsic work possible, i.e. if the compiler can do sinf for us.
rem GR turns off runtime types information
rem EHa turns off exception handling. No overhead for try/catch stuff
rem nologo disables the logo during compile NOTE(John): Removed this because I kinda like having it there
rem MT to include the C runtime library wholesale
rem FmWin32_handmade.map to craete a map file taht tells us where all the functions are in the exe.
rem opt:ref alters the .map file to prevent functions from being added that aren''t used