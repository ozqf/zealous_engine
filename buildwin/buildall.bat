
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
@call build_engine_lib.bat
@echo ---------------------------------------------------
@echo STEP 2 - build exe
@call build_exe_light.bat
@echo ---------------------------------------------------
@echo STEP 3 - Build plugins
@call build_plugins.bat
@echo ---------------------------------------------------
@echo STEP 4 - Build plugin sandbox
@call build_plugin_sandbox.bat
@echo ---------------------------------------------------
@echo STEP 5 - Build example 2d
@call build_example2d.bat
@echo ---------------------------------------------------
@echo STEP 6 - Build example 3d
@call build_example3d.bat
@echo ---------------------------------------------------
@echo STEP 7 - Build Run N Gun
@call build_run_n_gun.bat
@echo ---------------------------------------------------
@echo buildall done.
