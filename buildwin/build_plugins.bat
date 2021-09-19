@echo off

@echo --------------------------------------------------------
@echo Build Zealous Engine Plugins
cd..
if not exist bin mkdir bin
if not exist buildwin_plugins mkdir buildwin_plugins
cd buildwin_plugins

@rem Note that ofbx will spit out lots of warnings, so warnings as errors
@rem has to be disabled.

@rem Compile with no linking /c and no exceptions from extern "C" functions /EHsc
set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /O2 -Oi /c
@REM set compilerFlags=-nologo -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /c
set compilerDefines=/DPARANOID=1

set compilerInput1=../plugins/map_converter/zt_map_converter.cpp 
set compilerInput2=

@echo on
@cl %compilerFlags% %compilerDefines% %outputExe% %compilerInput1% %compilerInput2%
@lib  -nologo /out:map_converter.lib zt_map_converter.obj
@echo off

cd..
cd buildwin

@echo on
