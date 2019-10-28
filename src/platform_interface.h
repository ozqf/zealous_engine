#pragma once

#include "../common/com_defines.h"
#include "renderer_interface.h"

struct PlatformEventHeader
{
    i32 type;
    i32 size;
};

struct AppPlatform
{
    // Oops
    void                            (*Error)(char* msg, char* title);
    void                            (*Log)(char* msg);
    void                            (*Print)(char* msg);

    // Memory Allocation
    u8 (*Malloc)                    (MemoryBlock* mem, u32 bytesToAllocate);
    void (*Free)                    (MemoryBlock *mem);
    
    // Timing
    i64 (*SampleClock)             ();
    
    // Loading assets
    u32 (*MeasureFile)              (char* fileName);
    ErrorCode (*LoadFileIntoMemory) (char* fileName, u8* destination, u32 capacity);

    // Old Asset loading
    u8 (*LoadFileIntoHeap)          (Heap* heap, BlockRef* destRef, char* fileName, u8 assertOnFailure);
    //void (*LoadTexture)             (Heap* heap, BlockRef* destRef, char* path);
    Texture2DHeader* (*LoadTextureB) (Com_AllocateTexture callback, char* path);
    void (*BindTexture)             (void* rgbaPixels, u32 width, u32 height, u32 textureIndex);
    // Need to know the base directory for writing files
    i32 (*GetBaseDirectoryName)     (char* buffer, i32 bufferSize);
	
	// Writing Files
	i32 (*OpenFileForWriting)       (char* fileName);
    i32 (*WriteToFile)              (i32 fileId, u8* ptr, u32 numBytes);
	void (*SeekInFileFromStart)     (i32 fileId, u32 offset);
    i32 (*CloseFileForWriting)      (i32 fileId);
    void (*GetDateTime)             (DateTime* data);
    i32 (*SaveBMP)                  (Texture2DHeader* header);

    i32 (*WriteAllTextToFile)       (char* fileName, char* contents);

    // Input
    void (*SetMouseMode)            (enum ZMouseMode mode);

    // Commands
    void (*WriteTextCommand)        (char* ptr);

    // Rendering
    void (*RenderScene)             (RenderScene* scene);
    void (*SubmitRenderCommands)    (RenderCommand* commands, i32 numCommands);
    
    void (*LoadSound)               (u8* data, i32 numBytes);

    // Network
    i32  (*Init)                    ();
    i32  (*Shutdown)                ();
    i32  (*OpenSocket)              (u16 port, u16* portResult);
    i32  (*CloseSocket)             (i32 socketIndex);
    i32  (*Read)                    (i32 socketIndex, ZNetAddress* sender,  MemoryBlock* dataPtr);
    i32  (*SendTo)                  (i32 transmittingSocketIndex, ZNetAddress* address, u16 port, u8* data, i32 dataSize);

    //void (*Net_RunLoopbackTest) ();
};
