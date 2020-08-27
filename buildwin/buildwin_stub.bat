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
set in2=
set in3=
set in4=
set in5=
set in6=

@rem -- Main App input --
set in1=../src/app_stub/app_stub.cpp

@rem -- Stub App input --
@rem set in2=../src/app_stub/app_stub.cpp

set compIn=%in1%
@rem set compIn=../src/sim/sim_module.cpp

set compOut=/Fe../bin/stub/game.dll

@rem /EHsc to avoid exception handling issues.
@rem Warnings as Errors
set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /EHsc

@rem No warning elevation
@rem set compilerFlags=-nologo -MT -W4 -wd4100 -wd4201 -wd4189 /Zi /EHsc
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
@set in5=
@set in6=
@set compilerFlags=
@set compilerDefines=
@set linkInput=

@cd..
@cd buildwin

@echo on