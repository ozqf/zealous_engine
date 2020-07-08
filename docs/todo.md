# Zealous Engine Todo list

Note: This is not a road map and features may be listed but dropped. Items are not in any specific order

## Architectyre Notes

### Merge Client/Server modules

* Original plan of making this a networking test is kind of in the bin!
    * Merge separated client/server into a single 'game' module with 
        client/server as specific logic to manipulate the game instance.

### Minimum Viable Features Missing:

* Err, gameplay for one thing.
* App session control (loading new levels, ending games etc)
* Configuration files for binds and settings
* Basic graphics options (eg brightness/gamma)
* Sound - move over previous test with fmod
* Reading assets from pack files.

### Overall
* KISS, YAGNI, etc - reduce usage of function pointers and window DLL
* Merge Client and Server modules into single game module.

### Asset DB

Asset db access is currently unsafe between renderer and app.
Dirty assets are detected and updated at render start.
App could be updating and flagging geometry as dirty during render pass.

### Asset Generation

More routines to generate basic meshes and textures.

## Gameplay

### Enemies
* Flying version of Seeker

### Patterns
* A generic non-Sim module based pattern system!
* 3D circle, eg end of a cone, for spawning bullet 'hoops'

## Renderer
* UI
    * DONE Restore Forward renderer
    * DONE Straight-to-screen text
    * In-world (3D transformed) text.
* Billboard rendering and animation for sprite based objects
    * DONE Billboard ModelView transform and lighting
    * texture and sprite atlas creation + storage
    * animations - creation and playback
* Custom colour per object
* Implement proper GBuffer shader
* Clean up or remove wonky shadow mapping
