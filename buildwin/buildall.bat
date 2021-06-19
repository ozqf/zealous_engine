
cls
@echo ======================================================
@echo ================ ZEALOUS ENGINE ======================
@echo ======================================================

@echo STEP 1/6
@call buildwin_platform_libs.bat
@echo -
@echo - External libs built - Warnings as Errors start -
@echo -
@echo --------------------------------------------
@echo STEP 2/6
@call buildwin_platform.bat
@echo --------------------------------------------
@echo STEP 3/6
@call buildwin_window.bat
@echo --------------------------------------------
@echo STEP 4/6
@call buildwin_game.bat
@echo --------------------------------------------
@echo STEP 5/6
@call buildwin_game2d.bat
@echo --------------------------------------------
@echo STEP 6/6
@call buildwin_stub.bat
