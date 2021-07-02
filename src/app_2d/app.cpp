/*
App Stub - smallest possible app DLL implementation
*/
#include "../ze_common/ze_common.h"
#include "../zqf_renderer.h"
#include "../ze_module_interfaces.h"
#include "../sys_events.h"

struct Transform2D
{
	Vec3 pos;
	Vec2 scale;
	f32 radians;
};

internal ze_platform_export g_platform = {};

internal Vec3 g_pos = { 0.f, 0.f, 0.1f };
internal i32 g_buttons = 0;

internal Transform2D g_positions[256];
internal i32 g_numPositions = 0;
internal i32 g_maxPositions = 256;

internal void AddSprite(Vec3 pos, Vec2 scale, f32 radians)
{
	g_positions[g_numPositions] = {};
	g_positions[g_numPositions].pos = pos;
	g_positions[g_numPositions].scale = scale;
	g_positions[g_numPositions].radians = radians;
	g_numPositions++;
}

internal i32 AppImpl_WriteDraw(void* zrViewFrame)
{
	// reset objects
	g_numPositions = 0;
	AddSprite(g_pos, { 0.25f, 0.25f }, 0);
	AddSprite({ 0.5f, 0.5f, 0.1f }, { 0.25f, 0.5f }, 0);
	AddSprite({ -0.5f, -0.5f, 0.1f }, { 0.5f, 0.25f }, 0);
	
	
    ZRViewFrame* frame = (ZRViewFrame*)zrViewFrame;
	ZEBuffer* list = frame->list;
    // Add draw scenes and objects here...
	
	///////////////////////////////////////////////
    // Start a new scene...
	ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
	frame->numScenes += 1;
	// mark start of scene's objects list
	scene->params.objects = (ZRDrawObj*)list->cursor;
	scene->sentinel = ZR_SENTINEL;
    scene->params.projectionMode = ZR_PROJECTION_MODE_IDENTITY;
    Transform_SetToIdentity(&scene->params.camera);
    
    ///////////////////////////////////////////////
    // ...add objects...
	
	for (i32 i = 0; i < g_numPositions; ++i)
	{
		// alloc in object list buffer
		ZRDrawObj* drawObj = ZRDrawObj_InitInPlace(&list->cursor);
		scene->params.numObjects += 1;
		
		// configure
		Transform2D* t2d = &g_positions[i];
		drawObj->data.SetAsMesh(1, 0);
		drawObj->t.pos = t2d->pos;
		drawObj->t.scale = { t2d->scale.x, t2d->scale.y, 1 };
	}
	
	///////////////////////////////////////////////
	// ...Finish scene
    scene->params.numListBytes = list->cursor - (u8*)scene->params.objects;
    
    return ZE_ERROR_NONE;
}

/***************************************
* Module export functions
***************************************/
internal i32  AppImpl_Init()
{
	printf("App 2d Init\n");
	g_platform.SetMouseCaptured(NO);
	return ZE_ERROR_NONE;
}

internal i32  AppImpl_Shutdown()
{
    printf("App 2d Shutdown\n");
    // Free memory, assuming a new APP might be loaded in it's place
    return ZE_ERROR_NONE;
}

internal i32 AppImpl_RendererReloaded()
{
	return ZE_ERROR_NONE;
}

internal i32 AppImpl_ParseCommandString(const char* str, const char** tokens, const i32 numTokens)
{
	return NO;
}

internal i32 AppImpl_Tick(app_frame_info info)
{
    // -- do game logic --
	if (g_buttons & (1 << 0))
	{
		g_pos.x -= 1 * info.interval;
	}
	if (g_buttons & (1 << 1))
	{
		g_pos.x += 1 * info.interval;
	}
	ZE_ClampF32(&g_pos.x, -1, 1);
	ZE_ClampF32(&g_pos.y, -1, 1);
	
	// acquire and read platform events buffer
	ZEBuffer* events;
    g_platform.Acquire_EventBuffer(&events);
	
	// walk buffer
	u8* read = events->start;
    u8* end = events->cursor;
    i32 diff = end - read;
    if (diff == 0) { return ZE_ERROR_NONE; }
    
    while (read < end)
    {
        SysEvent* ev = (SysEvent*)read;
        read += ev->size;
        ErrorCode err = Sys_ValidateEvent(ev);
        if (err != ZE_ERROR_NONE)
        {
            ZE_ASSERT(NO, "Invalid system event");
        }
		if (ev->type == SYS_EVENT_INPUT)
		{
			// read input event
			SysInputEvent* input = (SysInputEvent*)ev;
			if (input->inputID == Z_INPUT_CODE_LEFT)
			{
				if (input->value != 0) { g_buttons |= (1 << 0); }
				else { g_buttons &= ~(1 << 0); }
			}
			if (input->inputID == Z_INPUT_CODE_RIGHT)
			{
				if (input->value != 0) { g_buttons |= (1 << 1); }
				else { g_buttons &= ~(1 << 1); }
			}
		}
	}
	
    events->Clear(NO);
    g_platform.Release_EventBuffer();

	return ZE_ERROR_NONE;
}

/***************************************
* Export Windows DLL functions
***************************************/
#include <Windows.h>

extern "C"
ze_app_export __declspec(dllexport) ZE_LinkToGameModule(ze_platform_export platform)
{
    printf("APP 2d linking to platform\n");
    g_platform = platform;
    ze_app_export appExport = {};
    appExport.Init = AppImpl_Init;
    appExport.Tick = AppImpl_Tick;
    appExport.WriteDraw = AppImpl_WriteDraw;
    appExport.ParseCommandString = AppImpl_ParseCommandString;
    appExport.RendererReloaded = AppImpl_RendererReloaded;
    appExport.Shutdown = AppImpl_Shutdown;
    appExport.sentinel = ZE_SENTINEL;
    return appExport;
}
