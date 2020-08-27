@cls
@echo --------------------------------------
@echo Copying to release
@echo off

cd..
@echo -- Purge release folder --
@rem delete release folder completely
if exist release RD /S /Q release
@rem recreate
mkdir release
@rem cd release
@rem del "x:\release\*" /q
@rem cd..

@rem copy
@rem engine
robocopy x:\bin x:\release zealous.exe /NJH /NJS
robocopy x:\bin x:\release glfw3.dll /NJH /NJS
robocopy x:\bin x:\release wingl.dll /NJH /NJS
@rem apps
robocopy x:\bin\base x:\release\base game.dll /NJH /NJS
robocopy x:\bin\stub x:\release\stub game.dll /NJH /NJS

cd buildwin
@echo on