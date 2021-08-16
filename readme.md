### Zealous Engine

#### About

* Game engine using opengl.
* Engine is in a single executable which will attach to a game DLL in a provided directory (like Quake 1/2/3). To write gamecode you just need to create the custom DLL, specifying the directory you want on the command line.
* Written in C++ but in an older C style. I have deliberately avoided using most C++ features.
* Inspired by old 90s FPS engines.
* Created for fun/learning and reference for other projects. I therefore doubt it will be of interest to anyone else!
* I don't really know what I'm doing...

#### Building

* Requires Visual C++ command line tools.
* Requires Node.js to run a script to embed shader text.
* Run buildall.bat.
* All libraries and data required to build the engine itself are embedded in the code or the repo, so it should be able to build and run in a 'stub' mode immediately (fingers crossed).
* Run r.bat to run the engine from the build directory.

#### 3rd Party Libraries

* GLFW for window
* GLAD for opengl
* stb_image.h for textures.
* OpenFBX for loading simple FBX model files.
