### Zealous Engine

## Modules
* platform (exe)
    * win_platform.cpp - Hosts window and game DLLs
* window (dll)
    * win_window.cpp - window, input, hosts renderer. Uses GLFW
    * zrgl.cpp - opengl renderer, uses GLAD
* game - (dll) Placed in base directory
    * app_module.cpp runs server and client and messaging between
    * server.cpp - main game logic and events
    * client.cpp - reads input and replicates server
    * client_render.cpp - builds draw list from client scene
    * sim_module.cpp - game state and shared logic between server and client
