@echo off

@echo --------------------------------------------------------
@echo Build Zealous Engine Example 3D

set buildDir=buildwin_example2d

cd..
if not exist bin mkdir bin
cd bin
if not exist example2d mkdir example2d
cd..
if not exist %buildDir% mkdir %buildDir%
cd %buildDir%
del *.* /Q
@rem === COMPILER SETTINGS ===
set outputExe=/Fe../bin/example2d/game.dll
@rem main compile flags, elevating warnings
@rem /LD - output a DLL
@REM set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /LD /O2 -Oi
set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /LD
@rem No elevated warnings
@rem set compilerFlags=-nologo -Gm -MT -W4 -wd4100 -wd4201 -wd4189 /Zi
set compilerDefines=/DPARANOID=1
@rem /DVERBOSE=1

@rem === DLL ===
@rem platform
set compIn1=../games/example2d/example2d.cpp

@echo on
@cl %compilerFlags% %compilerDefines% %outputExe% %compIn1%
@echo off

set compIn1=
set buildDir=

@cd..
@cd buildwin
@echo on
