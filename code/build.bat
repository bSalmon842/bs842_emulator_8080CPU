@echo off

REM -MTd for debug build
set commonFlagsCompiler= -MT -nologo -Gm- -GR- -EHa -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4244 -FC -Z7 -DEMU8080_INTERNAL=0 -DEMU8080_SLOW=0 -DEMU8080_WIN32=1
set commonFlagsLinker= -incremental:no -opt:ref user32.lib winmm.lib gdi32.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 32-bit
REM cl %commonFlagsCompiler% ..\code\win32_8080emu.cpp /link -subsystem:windows,5.1 %commonFlagsLinker%

cl %commonFlagsCompiler% ..\code\win32_8080emu.cpp /link %commonFlagsLinker%
popd