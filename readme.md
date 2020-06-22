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

## Random Notes

### Asset DB

Needs to Sit between renderer and app.
App
    * Load specific assets it wants to use.
    * Retrieve db asset handles to send to renderer.
Renderer
    * Take Asset Data, upload to GPU and assign GPU handle to that asset.
        - Callback to renderer?
    * Convert db Id to GPU handle.
    * Get list of currently un-uploading

