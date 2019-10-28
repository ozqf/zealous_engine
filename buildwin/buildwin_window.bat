@echo off

@echo --------------------------------------------------------
@echo Build Zealous Engine Windows Renderer
@echo --------------------------------------------------------

@echo Shaders
@node prep_shaders.js
@echo --------------------------------------------------------

@echo Code

cd..
if not exist bin mkdir bin
cd bin
if not exist base mkdir base
cd..

if not exist buildwin_renderer mkdir buildwin_renderer
cd buildwin_renderer
del *.* /q

set compilerFlags=-nologo -Gm -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /LD
set compilerDefines=/DPARANOID=1
@rem set linkInputA=opengl32.lib
@rem set linkInputB=user32.lib Gdi32.lib
set linkInputA=../lib/glfw3_vc2015/glfw3dll.lib
set linkInputB=../buildwin_platform_libs/platlibs.lib
set compInA=../src/win_window/win_window.cpp
set compInB=../src/zqf_renderer/zr_groups.cpp ../src/zqf_renderer/zrgl.cpp
set compInC=../src/zr_embedded/zr_embedded.cpp
set outputDLL=/Fe../bin/wingl.dll
@echo on
@cl %compilerFlags% %compilerDefines% %compInA% %compInB% %compInC% %outputDLL% %linkInputA% %linkInputB%
@echo off
set compilerFlags=
set compilerDefines=
set linkInputA=
set linkInputB=
set compilerInput=
set outputDLL=

cd..
cd buildwin
@echo on
