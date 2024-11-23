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
if not exist buildwin_platform mkdir buildwin_platform
cd buildwin_platform
del *.* /Q
@rem === COMPILER SETTINGS ===
set outputParam=/Fe../bin/zealous.exe
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
set linkStr=/link
set linkInputA=user32.lib opengl32.lib Gdi32.lib shell32.lib
@REM set linkInputB=../lib/fmod/fmod_vc.lib ../lib/fmod/fmodstudio_vc.lib
set linkInputB=
set linkInputC=../lib/glfw3_vc2019/glfw3_mt.lib ../buildwin_platform_libs/platlibs.lib
@rem @cl %compilerFlags% %compilerDefines% %outputParam% %compIn1% %compIn2% %compIn3% %compIn4% %compIn5% %linkStr% %linkInputA% %linkInputB% %linkInputC%

@rem --- build ---
@REM @echo --- build ---
@REM cl %compilerFlags% %compilerDefines% %compIn1% %compIn2% %compIn3% %compIn4% %compIn5% %linkStr% %linkInputA% %linkInputB% %linkInputC%


@echo --- build objs (no linking) ---
cl %compilerFlags% %compilerDefines% %compIn1% %compIn2% %compIn3% %compIn4% %compIn5% /c


@echo --------------------------------------------------------
@echo --- build lib ---
@echo on
lib -nologo /out:ze.lib command_console.obj config.obj win_main.obj win_window.obj zengine.obj ze_asset_db.obj ze_asset_loader.obj ze_debug.obj ze_embedded_assets.obj ze_events.obj ze_game_stub.obj ze_group_draw_items.obj ze_input.obj ze_opengl.obj ze_opengl_draw_sprites.obj ze_opengl_shaders.obj ze_scene.obj zrgl_data.obj zrgl_draw_mesh.obj zrgl_draw_primitives.obj zrgl_sandbox.obj zrgl_uploader.obj user32.lib opengl32.lib Gdi32.lib shell32.lib
@echo off



@rem @rem -- lib ---
@rem @set compIn1=win_main.obj win_window.obj
@rem @rem opengl renderer
@rem @set compIn2=ze_opengl.obj ze_opengl_shaders.obj zrgl_uploader.obj zrgl_draw_mesh.obj ze_opengl_draw_sprites.obj zrgl_draw_primitives.obj zrgl_data.obj zrgl_sandbox.obj
@rem @rem engine + services
@rem @set compIn3=zengine.obj config.obj ze_asset_db.obj ze_asset_loader.obj ze_debug.obj
@rem @set compIn4=ze_scene.obj ze_group_draw_items.obj ze_embedded_assets.obj ze_game_stub.obj
@rem @set compIn5=ze_input.obj command_console.obj ze_events.obj 
@rem @echo --- lib ---
@rem lib -nologo /out:ze.lib  %compIn1% %compIn2% %compIn3% %compIn4% %compIn5%

@rem --- link ---
@rem echo --- link ---
@rem @cl /link /out:ze.exe win_main.lib user32.lib opengl32.lib Gdi32.lib shell32.lib ../lib/glfw3_vc2019/glfw3_mt.lib ../buildwin_platform_libs/platlibs.lib
@rem cl -nologo /link /out:ze.exe win_main.lib
@rem -nologo win_main.lib /out:ze.exe

@rem echo --- lib ---
@rem lib  -nologo win /out:ze.lib

@set outputParam=
@set compilerFlags=
@set compilerDefines=
@set compIn1=
@set compIn2=
@set compIn3=
@set compIn4=
@set compIn5=

@set linkStr=
@set linkInputA=
@set linkInputB=
@set linkInputC=

@cd..
@cd buildwin
@echo on
