# Zealous Engine Todo list

Note: This is not a road map and features may be listed but dropped. Items are not in any specific order

## Architectyre Notes

### Overall
KISS, YAGNI, etc - reduce usage of function pointers and window DLL


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


## Gameplay

### Enemies
* Flying version of Seeker

### Patterns
* 3D circle, eg end of a cone, for spawning bullet 'hoops'

## Renderer
* UI
    * Restore Forward renderer
    * Straight-to-screen text
    * In-world (3D transformed) text.
* Billboard rendering and animation for sprite based objects
    * texture and sprite atlas creation + storage
    * animations - creation and playback
* Custom colour per object
* Implement proper GBuffer shader
* Clean up or remove wonky shadow mapping
