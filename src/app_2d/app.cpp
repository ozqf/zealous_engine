/*
App 2d - simple 2d game with orthographic sprites
*/
#include "../../headers/common/ze_common.h"
#include "../../headers/zqf_renderer.h"
#include "../../headers/ze_module_interfaces.h"
#include "../../headers/sys_events.h"
	
#include "game2d.h"	

struct Transform2D
{
	Vec3 pos;
	Vec2 scale;
	f32 radians;
};

internal ze_platform_export g_platform = {};

internal Vec2 g_mousePos = {};
internal Vec3 g_pos = {0.f, 0.f, 0.1f};
internal i32 g_buttons = 0;

internal Transform2D g_positions[256];
internal i32 g_numPositions = 0;
internal i32 g_maxPositions = 256;

internal i32 g_newId = 1;
internal ZEBlobStore g_objects = {};
internal i32 g_playerObjectId = 0;

internal void AddSprite(Vec3 pos, Vec2 scale, f32 radians)
{
	g_positions[g_numPositions] = {};
	g_positions[g_numPositions].pos = pos;
	g_positions[g_numPositions].scale = scale;
	g_positions[g_numPositions].radians = radians;
	g_numPositions++;
}

internal ZRDrawObj* AddSpriteB(Vec3 pos, Vec2 scale, f32 radians)
{
	i32 newId = g_newId++;
	u8 *mem = g_objects.GetFreeSlot(newId);
	if (mem == NULL)
	{
		printf("No free object slot\n");
		return NULL;
	}
	ZRDrawObj *drawObj = ZRDrawObj_InitInPlace(&mem);
	drawObj->userTag = newId;
	drawObj->data.SetAsMesh(1, 0);
	drawObj->t.pos = pos;
	drawObj->t.scale = {scale.x, scale.y, 1};
	return drawObj;
}

internal i32 AppImpl_WriteDraw(void *zrViewFrame)
{
	ZRViewFrame *frame = (ZRViewFrame *)zrViewFrame;
	G2d_Draw(frame);
	return ZE_ERROR_NONE;
}

internal i32 AppImpl_WriteDraw_defunct(void *zrViewFrame)
{
	// reset objects
	g_numPositions = 0;
	g_pos.z = -0.1f;
	AddSprite(g_pos, {0.25f, 0.25f}, 0);
	AddSprite({0.5f, 0.5f, 0.1f}, {0.5f, 0.5f}, 0);
	AddSprite({-0.5f, -0.5f, 0.1f}, {0.5f, 0.5f}, 0);

	ZRViewFrame *frame = (ZRViewFrame *)zrViewFrame;
	G2d_Draw(frame);
	ZEBuffer *list = frame->list;
	// Add draw scenes and objects here...

	///////////////////////////////////////////////
	// Start a new scene...
	ZRSceneFrame *scene;
	scene = ZRScene_InitInPlace(frame->list, ZR_PROJECTION_MODE_ORTHO_BASE, false);
	frame->numScenes += 1;
	// mark start of scene's objects list
	scene->params.objects = (ZRDrawObj *)list->cursor;
	Transform_SetToIdentity(&scene->params.camera);

	///////////////////////////////////////////////
	// ...add objects...

	ZRDrawObj *drawObj;
	i32 count = g_objects.Count();
	for (i32 i = 0; i < count; ++i)
	{
		drawObj = (ZRDrawObj*)g_objects.GetByIndex(i);
		ZRDrawObj *target = (ZRDrawObj *)list->cursor;
		*target = *drawObj;
		// advance cursor and count
		list->cursor += sizeof(ZRDrawObj);
		scene->params.numObjects += 1;
	}
	#if 0
	Transform2D *t2d;
	for (i32 i = 0; i < g_numPositions; ++i)
	{
		// alloc in object list buffer
		drawObj = ZRDrawObj_InitInPlace(&list->cursor);
		scene->params.numObjects += 1;

		// configure
		t2d = &g_positions[i];
		drawObj->data.SetAsMesh(1, 0);
		drawObj->t.pos = t2d->pos;
		drawObj->t.scale = {t2d->scale.x, t2d->scale.y, 1};
	}
	#endif

	if (g_playerObjectId != 0)
	{
		ZRDrawObj *plyr = (ZRDrawObj *)g_objects.GetById(g_playerObjectId);
		plyr->t.pos = g_pos;
	}
	

	// Cursor - event position
	// alloc in object list buffer
	drawObj = ZRDrawObj_InitInPlace(&list->cursor);
	scene->params.numObjects += 1;
	Vec3 scale = {2.f / 16.f, 2.f / 16.f, 1.f};
	// Vec3 scale = {1, 1, 1};
	// Vec3 scale = {0.5f, 0.5f, 1.f};
	f32 z = 0.4f;

	Vec2 mPos = g_platform.GetNormalisedMousePos();
	// configure
	drawObj->data.SetAsMesh(1, 2);
	drawObj->t.pos = {mPos.x * (16.f / 9.f), mPos.y, z};
	drawObj->t.scale = scale;

	// cursor - immediately position lookup
	drawObj = ZRDrawObj_InitInPlace(&list->cursor);
	scene->params.numObjects += 1;

	// configure
	drawObj->data.SetAsMesh(1, 3);
	drawObj->t.pos = {g_mousePos.x * (16.f / 9.f), g_mousePos.y, z};
	drawObj->t.scale = scale;

	///////////////////////////////////////////////
	// ...Finish scene
	scene->params.numListBytes = list->cursor - (u8 *)scene->params.objects;

	return ZE_ERROR_NONE;
}

/***************************************
* Module export functions
***************************************/
internal i32 AppImpl_Init()
{
	printf("App 2d Init\n");
	g_platform.SetMouseCaptured(NO);
	G2d_Init();
	ZE_InitBlobStore(&g_objects, 1024, sizeof(ZRDrawObj), 0);
	AddSpriteB({0.5f, 0.5f, 0.1f}, {0.5f, 0.5f}, 0);
	AddSpriteB({-0.5f, -0.5f, 0.1f}, {0.5f, 0.5f}, 0);

	ZRDrawObj *plyr = AddSpriteB({0.f, 0.f, 0.1f}, {0.25f, 0.25f}, 0);
	if (plyr != NULL)
	{
		g_playerObjectId = plyr->userTag;
	}

	return ZE_ERROR_NONE;
}

internal i32 AppImpl_Shutdown()
{
	printf("App 2d Shutdown\n");
	G2d_Shutdown();
	return ZE_ERROR_NONE;
}

internal i32 AppImpl_RendererReloaded()
{
	return ZE_ERROR_NONE;
}

internal i32 AppImpl_ParseCommandString(const char *str, const char **tokens, const i32 numTokens)
{
	return NO;
}

internal i32 AppImpl_Tick(app_frame_info info)
{
	printf("App tick - %d\n", info.frameNumber);
	// acquire and read platform events buffer
	ZEBuffer *events;
	g_platform.Acquire_EventBuffer(&events);

	// walk buffer
	u8 *read = events->start;
	u8 *end = events->cursor;
	i32 diff = end - read;
	if (diff == 0)
	{
		return ZE_ERROR_NONE;
	}

	while (read < end)
	{
		SysEvent *ev = (SysEvent *)read;
		read += ev->size;
		ErrorCode err = Sys_ValidateEvent(ev);
		if (err != ZE_ERROR_NONE)
		{
			ZE_ASSERT(NO, "Invalid system event");
		}
		if (ev->type == SYS_EVENT_INPUT)
		{
			// read input event
			SysInputEvent *input = (SysInputEvent *)ev;
			i32 id = input->inputID;
			if (id == Z_INPUT_CODE_LEFT)
			{
				if (input->value != 0)
				{
					g_buttons |= (1 << 0);
				}
				else
				{
					g_buttons &= ~(1 << 0);
				}
			}
			if (id == Z_INPUT_CODE_RIGHT)
			{
				if (input->value != 0)
				{
					g_buttons |= (1 << 1);
				}
				else
				{
					g_buttons &= ~(1 << 1);
				}
			}
			if (id == Z_INPUT_CODE_MOUSE_POS_X)
			{
				g_mousePos.x = input->normalised;
			}
			if (id == Z_INPUT_CODE_MOUSE_POS_Y)
			{
				g_mousePos.y = input->normalised;
			}
			if (id == Z_INPUT_CODE_MOUSE_1)
			{
				printf("Shoot\n");
			}
		}
	}

	events->Clear(NO);
	g_platform.Release_EventBuffer();

	// -- do game logic --
	G2d_Tick(info.interval);
	
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

	g_objects.Truncate();

	return ZE_ERROR_NONE;
}

/***************************************
* Export Windows DLL functions
***************************************/
#include <Windows.h>

extern "C" ze_app_export __declspec(dllexport) ZE_LinkToGameModule(ze_platform_export platform)
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
