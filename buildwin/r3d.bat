@echo off
cd..
cd bin
@echo on

@rem fullscreen, no console, nadda
@rem start zealous.exe

@rem windowed, logging
start zealous.exe -w -c -l -g example3d

@rem pause immediately 
@REM start zealous.exe -w -c -l --pauseonstart

@echo off
cd..
cd buildwin
@echo on