
cls
@echo ======================================================
@echo ================ ZEALOUS ENGINE ======================
@echo ======================================================

@echo STEP 1 - platform libs
@call build_platform_libs.bat
@echo ---------------------------------------------------
@echo - External libs built - Warnings as Errors start -
@echo ---------------------------------------------------
@echo STEP 2 - build engine
@call build_exe.bat
@echo ---------------------------------------------------
@echo STEP 3 - Build example 3d
@call build_example3d.bat
@echo ---------------------------------------------------
@echo buildall done.
