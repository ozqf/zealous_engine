
@echo off

echo .
echo === BUILD PLATFORM LIBS ===
cd..
if not exist bin mkdir bin
if not exist buildwin_platform_libs mkdir buildwin_platform_libs
cd buildwin_platform_libs

@rem Note that ofbx will spit out lots of warnings, so warnings as errors
@rem has to be disabled.

@rem Compile with no linking /c and no exceptions from extern "C" functions /EHsc
set compilerFlags=-nologo -MT -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /c /EHsc
set compilerDefines=/D_WIN32

set compilerInput1=../lib/glad/glad.c 
set compilerInput2=../lib/openfbx/ofbx.cpp ../lib/openfbx/miniz.c

@echo on
cl %compilerFlags% %compilerDefines% %outputExe% %compilerInput1% %compilerInput2%
lib  -nologo /out:platlibs.lib glad.obj miniz.obj ofbx.obj
@echo off

cd..
cd buildwin

@echo on
