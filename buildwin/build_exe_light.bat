@echo off

@echo --------------------------------------------------------
@echo Build Zealous Engine Windows Executable

@echo Embed Engine Shaders
@call write_engine_shaders.bat
@echo Embed Renderer Sandbox Shaders
@call write_sandbox_shaders.bat
@echo --------------------------------------------------------

cd..
if not exist bin mkdir bin
if not exist buildwin_exe mkdir buildwin_exe
cd buildwin_exe
del *.* /Q
@rem === COMPILER SETTINGS ===
set outputExe=/Fe../bin/zealous.exe
@rem main compile flags, elevating warnings
@REM set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi /O2 -Oi
set compilerFlags=-nologo -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi
@rem No elevated warnings
@rem set compilerFlags=-nologo -Gm -MT -W4 -wd4100 -wd4201 -wd4189 /Zi
set compilerDefines=/DPARANOID=1 /DGLFW_USE_HYBRID_HPG
@rem /DVERBOSE=1

@rem === Compile Win32 Window application ===
@rem platform
set compIn1=../engine/src/windows/win_main.cpp ../engine/src/windows/win_window.cpp
@rem opengl renderer
set compIn2=../engine/src/opengl/ze_opengl.cpp ../engine/src/opengl/ze_opengl_shaders.cpp ../engine/src/opengl/zrgl_uploader.cpp ../engine/src/opengl/draw/zrgl_draw_mesh.cpp ../engine/src/opengl/draw/ze_opengl_draw_sprites.cpp ../engine/src/opengl/draw/zrgl_draw_primitives.cpp ../engine/src/opengl/zrgl_data.cpp ../engine/src/opengl/sandbox/zrgl_sandbox.cpp
@rem engine + services
set compIn3=../engine/src/zengine.cpp ../engine/src/config/config.cpp ../engine/src/assetdb/ze_asset_db.cpp ../engine/src/assetdb/ze_asset_loader.cpp ../engine/src/debug/ze_debug.cpp
set compIn4=../engine/src/scene/ze_scene.cpp ../engine/src/scene/ze_group_draw_items.cpp ../engine/src/embedded_assets/ze_embedded_assets.cpp ../engine/src/game_stub/ze_game_stub.cpp
set compIn5=../engine/src/input/ze_input.cpp ../engine/src/command_console/command_console.cpp ../engine/src/events/ze_events.cpp

@rem === Compile Testing Win32 Console application
@rem set compilerInput=../src/Platform/win32_consoleApp.cpp
@rem set linkStr=/link /SUBSYSTEM:CONSOLE

@rem === LINK SETTINGS === (disable if running win32 console application test)
set linkInputA=user32.lib opengl32.lib Gdi32.lib shell32.lib
@REM set linkInputB=../lib/fmod/fmod_vc.lib ../lib/fmod/fmodstudio_vc.lib
set linkInputB=../buildwin_platform/ze.lib
set linkInputC=../lib/glfw3_vc2019/glfw3_mt.lib ../buildwin_platform_libs/platlibs.lib
@echo on
@cl %compilerFlags% %compilerDefines% %outputExe% ../engine/src/windows/win_main.cpp /link %linkInputB% %linkInputC%
@echo off
set outputExe=
set compilerFlags=
set compilerDefines=
set compIn1=
set compIn2=
set compIn3=
set compIn4=
set compIn5=

set linkInputA=
set linkInputB=
set linkInputC=

@cd..
@cd buildwin
@echo on
