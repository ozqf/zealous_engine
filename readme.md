### Zealous Engine

* Game engine using opengl.
* Inspired by old 90s FPS engines.
* Created for fun/learning and reference for other projects.
* Written in C++ but in an older C style. I have deliberately avoided using most C++ features.

## Build Process

Requires visual studio command line compile tools and node js
* I have attempted to keep third party code isolated from the engine's core modules.
* Third party libraries are included in source so it should build as long as vs compiler is ready.
* node js used to generate lib/shaders.h which embeds .glsl shader files.


## Modules
* platform (exe)
    * win_platform.cpp - Hosts window and game DLLs
    * zr_asset_db.cpp - a 'database' of loaded textures/meshes/etc. Shared by renderer and app.
    * ze_win_socket.cpp - winsock
    * zr_embedded.cpp - embedded mesh and texture assets, loaded automatically by asset db
* window (dll)
    * win_window.cpp - window, input, hosts renderer. Uses GLFW
    * zui.cpp - (very) basic UI widgets.
    * zrgl.cpp - opengl renderer, uses GLAD
    * zr_groups.cpp - non-API specific renderer logic
* game - (dll) Placed in base directory
    * app_module.cpp runs server and client and messaging between
    * server.cpp - main game logic and events
    * client.cpp - reads input and replicates server
    * client_render.cpp - builds draw list from client scene
    * sim_module.cpp - game state and shared logic between server and client

## 3rd Party Libraries:

* GLFW for window
* GLAD for opengl
* stb_image.h for textures.
* OpenFBX for loading FBX model files.
