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
* Run buildall.bat to build everything. other build batch files build specific components, for example engine, engine libs, example games etc.
* All libraries and asset data required to build the engine itself are embedded in the code or the repo, so it should be able to build and run in a 'stub' mode immediately (fingers crossed). Apologies if not.

#### Running
* Run r.bat to run the engine from the build directory. Other batch files can be found to run example games (eg r2d and r3d will run the 2D and 3D example games).
* The game will look for a game dll in the 'base' directory by default. Change  ```-g MySpecificDirectory``` or ```--game MySpecificDirectory``` on the command line.
* If no dll is found, the game will load a test game 'stub' so at least you see something...

#### 3rd Party Libraries

* GLFW for window
* GLAD for opengl
* stb_image.h for textures.
* OpenFBX for loading simple FBX model files.
* Box2d - 2d physics
* Bullet - 3d physics
* fmod - sound

#### Console Commands and variables

TODO - ...implement the command console....

#### Command line options

eg ```zealous.exe -g my_game_dir -w --monitor 2```

##### Functionality parameters

```-g <my_game_sub_directory>``` or ```--game <my_game_sub_directory>```
Specify the directory containing the game.dll to load.

```-w``` or ```--window```
Start in a window rather than default which is borderless fullsize window.

```--monitor <number>```
eg --monitor 0, --monitor 1 etc
Specify which monitor to create the window on.

##### Debugging parameters

```-c``` or ```--console```
Create command prompt window for debugging

```-l``` or ```--log```
Start a logging file

```--single```
Start up, run one frame, take a screenshot and exit

```--pauseonstart``` and ```--pauseonstop```
Pauses execution at startup or shutdown with an alert popup
