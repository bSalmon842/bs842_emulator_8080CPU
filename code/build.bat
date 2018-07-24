@echo off

REM -MTd for debug build
set commonFlagsCompiler= -MTd -nologo -Gm- -GR- -EHa -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4244 -FC -Z7 -DEMU8080_INTERNAL=1 -DEMU8080_SLOW=1 -DEMU8080_WIN32=1
set commonFlagsLinker= -incremental:no -opt:ref

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 32-bit
REM cl %commonFlagsCompiler% ..\code\win32_8080emu.cpp /link -subsystem:windows,5.1 %commonFlagsLinker%

cl %commonFlagsCompiler% ..\code\win32_8080emu.cpp -Fmwin32_8080emu.map /link %commonFlagsLinker%
popd