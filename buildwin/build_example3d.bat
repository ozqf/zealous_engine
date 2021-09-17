@echo off

@echo --------------------------------------------------------
@echo Build Zealous Engine Example 3D

set buildDir=buildwin_example3d

cd..
if not exist bin mkdir bin
cd bin
if not exist example3d mkdir example3d
cd..
if not exist %buildDir% mkdir %buildDir%
cd %buildDir%
del *.* /Q
@rem === COMPILER SETTINGS ===
set outputExe=/Fe../bin/example3d/game.dll
@rem main compile flags, elevating warnings
@rem /LD - output a DLL
set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /LD
@rem No elevated warnings
@rem set compilerFlags=-nologo -Gm -MT -W4 -wd4100 -wd4201 -wd4189 /Zi
set compilerDefines=/DPARANOID=1
@rem /DVERBOSE=1

@rem === DLL ===
@rem platform
set compIn1=../games/example3d/example3d.cpp

set linkInA=../buildwin_plugins/map_converter.lib

@echo on
cl %compilerFlags% %compilerDefines% %outputExe% %compIn1% /link %linkInA%
@echo off

set compIn1=
set linkInA=
set buildDir=

@cd..
@cd buildwin
@echo on
