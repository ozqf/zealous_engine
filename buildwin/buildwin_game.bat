@echo off

@rem echo Setup vs
@rem @call vsvars2015

@echo --------------------------------------------------------
@echo Build Zealous Engine Game dll

cd..
if not exist bin mkdir bin
cd bin
if not exist base mkdir base
cd..

if not exist buildwin_game mkdir buildwin_game
cd buildwin_game
del *.* /q

@rem -- Common module input --
@rem set in1=../src/common/com_module.cpp
set in1=

@rem -- Main App input --
@rem set in2=../src/app/app_module.cpp ../src/app/app_textures.cpp
set in1=../src/app_base/client/client_render.cpp ../src/ui/zui.cpp
set in2=../src/app_base/app_module.cpp ../src/app_base/app_ui.cpp
@rem set in3=../src/sim/sim_module.cpp ../src/network/znet_module.cpp
set in3=../src/sim/sim_module.cpp ../src/voxel_world/voxel_world.cpp
set in4=../src/app_base/server/server.cpp ../src/app_base/client/client.cpp
@rem set in5=../src/physics/bullet_wrapper/bullet_module.cpp
set in5=../src/network/zqf_network_module.cpp

@rem -- Stub App input --
@rem set in2=../src/app_stub/app_stub.cpp ../src/app/app_textures.cpp

set compIn=%in1% %in2% %in3% %in4% %in5%
@rem set compIn=../src/sim/sim_module.cpp

set compOut=/Fe../bin/base/game.dll

@rem /EHsc to avoid exception handling issues.
@rem Warnings as Errors
set compilerFlags=-nologo -Gm -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /EHsc

@rem No warning elevation
@rem set compilerFlags=-nologo -Gm -MT -W4 -wd4100 -wd4201 -wd4189 /Zi /EHsc
set compilerDefines=/DPARANOID=1
@rem set linkInput=../lib/bullet/ZBulletPhysicsWrapper.lib
set linkInput=
@echo on
@cl %compilerFlags% %compilerDefines% /LD %compIn% %compOut% %linkInput%

@set compIn=
@set compOut=
@set in1=
@set in2=
@set in3=
@set in4=
@set compilerFlags=
@set compilerDefines=
@set linkInput=

@cd..
@cd buildwin

@echo on