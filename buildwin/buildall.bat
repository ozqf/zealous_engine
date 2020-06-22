
cls
@echo ======================================================
@echo ================ ZEALOUS ENGINE ======================
@echo ======================================================

@call buildwin_platform_libs.bat
@echo -
@echo - External libs built - Warnings as Errors start -
@echo -
@call buildwin_platform.bat
@call buildwin_window.bat
@call buildwin_game.bat

