@echo off

@echo --------------------------------------------------------
@echo Build Run N Gun game lib

set buildDir=buildwin_game

cd..
if not exist bin mkdir bin
cd bin
if not exist run_n_gun mkdir run_n_gun
cd..
if not exist %buildDir% mkdir %buildDir%
cd %buildDir%
del *.* /Q
@rem === COMPILER SETTINGS ===
set outputExe=/Fe../bin/run_n_gun/game.dll
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
set compIn1=../games/run_n_gun/*.cpp
set compIn2=../plugins/map2d/*.cpp
set compIn3=../plugins/physics2d/*.cpp

set linkInA=../lib/box2d/box2d.lib

@echo on
@cl %compilerFlags% %compilerDefines% %outputExe% %compIn1% %compIn2% %compIn3% /link %linkInA%
lib  -nologo /out:game.lib *.obj
@echo off

set compIn1=
set compIn2=
set compIn3=
set linkInA=
set buildDir=

@echo ----------------------------------------------
@echo --- Build Run N Gun executable

set outputExe=/Fe../bin/rng.exe
set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi
set compilerDefines=/DPARANOID=1 /DGLFW_USE_HYBRID_HPG
set linkInputA=user32.lib opengl32.lib Gdi32.lib shell32.lib
set linkInputB=../lib/glfw3_vc2019/glfw3_mt.lib ../buildwin_platform_libs/platlibs.lib
set linkInputC=../buildwin_platform/ze.lib ../buildwin_game/game.lib ../lib/box2d/box2d.lib
@echo on
@cl %compilerFlags% %compilerDefines% %outputExe% ../engine/src/windows/win_main.cpp /link %linkInputA% %linkInputB% %linkInputC%
@echo off


@cd..
@cd buildwin
@echo on
