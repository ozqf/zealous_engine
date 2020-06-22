@echo off

@echo --------------------------------------------------------
@echo Build Zealous Engine Windows Executable

cd..
if not exist bin mkdir bin
if not exist buildwin_platform mkdir buildwin_platform
cd buildwin_platform
del *.* /Q
@rem === COMPILER SETTINGS ===
set outputExe=/Fe../bin/zealous.exe
@rem main compile flags, elevating warnings
set compilerFlags=-nologo -Gm -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi
@rem No elevated warnings
@rem set compilerFlags=-nologo -Gm -MT -W4 -wd4100 -wd4201 -wd4189 /Zi
set compilerDefines=/DPARANOID=1
@rem /DVERBOSE=1

@rem === Compile Win32 Window application
set compIn1=../src/win_platform/win_platform.cpp ../src/assetdb/zr_asset_db.cpp
set compIn2=../src/win_platform/ze_win_socket.cpp ../src/zr_embedded/zr_embedded.cpp

@rem === Compile Testing Win32 Console application
@rem set compilerInput=../src/Platform/win32_consoleApp.cpp
@rem set linkStr=/link /SUBSYSTEM:CONSOLE

@rem === LINK SETTINGS === (disable if running win32 console application test)
set linkStr=/link
set linkInputA=user32.lib opengl32.lib Gdi32.lib  Ws2_32.lib
@echo on
@cl %compilerFlags% %compilerDefines% %outputExe% %compIn1% %compIn2% %linkStr% %linkInputA%
@echo off
set outputExe=
set compilerFlags=
set compilerDefines=
set compIn1=
set compIn2=

set linkStr=
set linkInputA=
set linkInputB=
set linkInputC=

@cd..
@cd buildwin
@echo on

@rem Project defines
@rem /DPARANOID=1 -> enable all asserts

@rem === COMPILER FLAGS ===
@rem -WX -> warnings as errors
@rem -W4 -> Max warning level that's sane
@rem -Oi -> enable compiler 'intrinsics'
@rem -GR -> turn off runtime type information (C++ feature)
@rem -EHa -> turn off exception handling
@rem -nologo -> disable compile command line header
@rem -wd4100 -> disable warning
@rem -wd4201 -> disable warning
@rem -wd4189 -> disable warning
@rem /Zi -> generate debug information
@rem /Fe -> specify file name and path
@rem -subsystem:windows,5.1 creates xp compatible windows program
@rem -MT package standard library into exe (increase compatibility)
@rem -Gm switch off minimal rebuild stuff (not using any of it)
@rem -Fm Create map file (contains addresses of all variables/functions)
@rem -opt:ref -> make compiler aggressive in removal of unused code
@rem /LD -> compile to DLL
