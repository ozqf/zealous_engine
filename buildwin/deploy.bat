@cls
@echo --------------------------------------
@echo Copying to release
@echo off

cd..
if exist release RD /S /Q release

mkdir release
@rem cd release
@echo -- Purge release folder --
del "x:\release\*" /q
@rem cd..
robocopy x:\bin x:\release zealous.exe /NJH /NJS
robocopy x:\bin x:\release glfw3.dll /NJH /NJS
robocopy x:\bin x:\release wingl.dll /NJH /NJS
robocopy x:\bin\base x:\release\base game.dll /NJH /NJS

cd buildwin
@echo on