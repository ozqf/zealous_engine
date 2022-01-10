@echo off

@echo --------------------------------------------------------
@echo Build Run N Gun game example

set buildDir=buildwin_run_n_gun

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
set compIn1=../games/run_n_gun/run_n_gun.cpp ../games/run_n_gun/rng_sim.cpp
set compIn2=../plugins/physics2d/ze_physics2d.cpp ../games/run_n_gun/tile_map.cpp

set linkInA=../lib/box2d/box2d.lib

@echo on
cl %compilerFlags% %compilerDefines% %outputExe% %compIn1% %compIn2% /link %linkInA%
@echo off

set compIn1=
set compIn2=
set linkInA=
set buildDir=

@cd..
@cd buildwin
@echo on
