#include "rng_internal.h"

#define ED_MOVE_UP "ed_move_up"
#define ED_MOVE_DOWN "ed_move_down"
#define ED_MOVE_LEFT "ed_move_left"
#define ED_MOVE_RIGHT "ed_move_right"
#define ED_MOUSE_1 "ed_mouse_1"
#define ED_MOUSE_2 "ed_mouse_2"

#define ED_MODIFIER_1 "ed_modifier_1"
#define ED_DELETE "ed_delete"

#define ED_SOLID_GEOMETRY_MODE "ed_solid_geometry_mode"
#define ED_LINE_GEOMETRY_MODE "ed_line_geometry_mode"
#define ED_ENTITY_MODE "ed_entity_mode"

#define ED_NEXT_ENTITY_TYPE

#define DRAG_CORNER_NONE 0
#define DRAG_CORNER_NW 1
#define DRAG_CORNER_NE 2
#define DRAG_CORNER_SE 3
#define DRAG_CORNER_SW 4

struct WorldVolume
{
	i32 id;
	Point2 gridMin;
	Point2 gridMax;
	zeHandle drawId;
	i32 bSelected;
	i32 bHighlighted;

	// for finding corner selection when dragging
	// only here for debugging.
	f32 fracX;
	f32 fracY;
};

struct EdInput
{
	u8 moveUp;
	u8 moveDown;
	u8 moveLeft;
	u8 moveRight;
	
	u8 mouse1;
	u8 mouse2;
	
	u8 modifier1;
	u8 del;

	u8 mode1;
	u8 mode2;
	u8 mode3;
};

struct EdTickData
{
	float delta;
	EdInput input;
	EdInput lastInput;
	Vec2 mouseScreenPos;
	frameInt frameNumber;
};

ze_internal ZEngine g_engine;
ze_internal zeHandle g_edScene = ZE_EMPTY_HANDLE;
ze_internal zeHandle g_pointCursor = ZE_EMPTY_HANDLE;
ze_internal zeHandle g_gridCursor = ZE_EMPTY_HANDLE;
ze_internal ZEBlobStore g_volumes = {};
ze_internal EdTickData g_tick = {};
ze_internal i32 g_nextVolumeId = 1;
ze_internal i32 g_editMode = 0;
ze_internal i32 g_selectedVolumeId = 0;
ze_internal Vec2 g_mouseWorldPos;
ze_internal i32 g_dragCorner = 0;

ze_internal ZEBuffer g_debugText;

ze_internal Point2 PosToGrid(Vec2 pos)
{
	Point2 p = {};
	p.x = int(ZFLOORF(pos.x));
	p.y = int(ZFLOORF(pos.y));
	return p;
}

ze_internal Point2 GetVolumeSize(WorldVolume* vol)
{
	Point2 p;
	// minimum size of 1 cell in each axis.
	p.x = (vol->gridMax.x - vol->gridMin.x) + 1;
	p.y = (vol->gridMax.y - vol->gridMin.y) + 1;
	return p;
}

ze_internal Vec2 GetVolumeCentre(WorldVolume* vol)
{
	Vec2 size = Vec2_FromPoint2(GetVolumeSize(vol));
	Vec2 centre;
	centre.x = (f32)vol->gridMin.x + (size.x * 0.5f);
	centre.y = (f32)vol->gridMin.y + (size.y * 0.5f);
	return centre;
}

ze_internal void RefreshVolume(WorldVolume* vol)
{
	f32 minX = (f32)vol->gridMin.x, minY = (f32)vol->gridMin.y;
	f32 width = (f32)vol->gridMax.x - minX;
	width += 1;
	f32 height = (f32)vol->gridMax.y - minY;
	height += 1;

	Vec2 centre = GetVolumeCentre(vol);

	// set position and scale
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_edScene, vol->drawId);
	// obj->t.pos.x = minX + 0.5f;
	// obj->t.pos.y = minY + 0.5f;
	obj->t.pos.x = centre.x;
	obj->t.pos.y = centre.y;
	obj->t.scale = { width * 0.5f, height * 0.5f, 1 };
	vol->bSelected
		? obj->data.quad.colour = COLOUR_F32_YELLOW
		: obj->data.quad.colour = COLOUR_F32_LIGHT_GREY;
}

ze_internal void SetSelectedVolume(i32 id)
{
	WorldVolume* vol = NULL;
	if (g_selectedVolumeId == id)
	{
		return;
	}
	if (g_selectedVolumeId != 0)
	{
		vol = (WorldVolume*)g_volumes.GetById(g_selectedVolumeId);
		vol->bSelected = NO;
		RefreshVolume(vol);
		vol = NULL;
		g_selectedVolumeId = 0;
	}

	if (id != 0)
	{
		g_selectedVolumeId = id;
		vol = (WorldVolume*)g_volumes.GetById(id);
		vol->bSelected = YES;
	}
	if (vol != NULL)
	{
		RefreshVolume(vol);
	}
}

ze_internal void AddVolumeAt(Point2 gridPos)
{
	RNGPRINT("Add volume at %d, %d\n", gridPos.x, gridPos.y);
	i32 newId = g_nextVolumeId++;
	WorldVolume* vol = (WorldVolume*)g_volumes.GetFreeSlot(newId);
	*vol = {};
	vol->id = newId;
	vol->gridMin = gridPos;
	vol->gridMax = gridPos;
	ZRDrawObj* obj = g_engine.scenes.AddFullTextureQuad(
		g_edScene, FALLBACK_TEXTURE_WHITE, { 1.f, 1.f}, COLOUR_F32_LIGHT_GREY);
	vol->drawId = obj->id;
	SetSelectedVolume(vol->id);
}

ze_internal void RemoveVolume(i32 id)
{
	WorldVolume* vol = (WorldVolume*)g_volumes.GetById(id);
	if (vol == NULL) { return; }
	g_engine.scenes.RemoveObject(g_edScene, vol->drawId);
	g_volumes.MarkForRemoval(id);
}

ze_internal i32 TestGridPosVsVolume(Point2 gridPos, WorldVolume* vol)
{
	ZE_ASSERT(vol, "Volume is null");
	if (gridPos.x < vol->gridMin.x) { return NO; }
	if (gridPos.x > vol->gridMax.x) { return NO; }
	if (gridPos.y < vol->gridMin.y) { return NO; }
	if (gridPos.y > vol->gridMax.y) { return NO; }
	return YES;
}

ze_internal WorldVolume* FindVolumeAt(Point2 g)
{
	i32 num = g_volumes.Count();
	for (i32 i = 0; i < num; ++i)
	{
		WorldVolume* vol = (WorldVolume*)g_volumes.GetByIndex(i);
		if (vol == NULL) { continue; }
		if (TestGridPosVsVolume(g, vol))
		{
			return vol;
		}
	}
	return NULL;
}

ze_internal i32 SelectDragCorner(Vec2 mouseWorldPos, Point2 gridPos, WorldVolume* vol)
{
	Vec2 centre = GetVolumeCentre(vol);
	f32 fracX = mouseWorldPos.x - (f32)centre.x;
	f32 fracY = mouseWorldPos.y - (f32)centre.y;
	vol->fracX = fracX;
	vol->fracY = fracY;
	if (fracX < 0)
	{
		if (fracY > 0) { return DRAG_CORNER_NW; }
		else { return DRAG_CORNER_SW; }
	}
	else
	{
		if (fracY > 0) { return DRAG_CORNER_NE; }
		else { return DRAG_CORNER_SE; }
	}
}

ze_internal void MouseDragVolume(Vec2 mouseWorldPos, Point2 gridPos, WorldVolume* vol)
{
	switch (g_dragCorner)
	{
		case DRAG_CORNER_NE:
		// +x, +y
		if (gridPos.x >= vol->gridMin.x)
		{
			vol->gridMax.x = gridPos.x;
		}
		if (gridPos.y >= vol->gridMin.y)
		{
			vol->gridMax.y = gridPos.y;
		}
		break;
		case DRAG_CORNER_NW:
		// -x, +y
		if (gridPos.x <= vol->gridMax.x)
		{
			vol->gridMin.x = gridPos.x;
		}
		if (gridPos.y >= vol->gridMin.y)
		{
			vol->gridMax.y = gridPos.y;
		}
		break;
		case DRAG_CORNER_SE:
		// +x, -y
		if (gridPos.x >= vol->gridMin.x)
		{
			vol->gridMax.x = gridPos.x;
		}
		if (gridPos.y <= vol->gridMax.y)
		{
			vol->gridMin.y = gridPos.y;
		}
		break;
		case DRAG_CORNER_SW:
		// -x, -y
		if (gridPos.x <= vol->gridMax.x)
		{
			vol->gridMin.x = gridPos.x;
		}
		if (gridPos.y <= vol->gridMax.y)
		{
			vol->gridMin.y = gridPos.y;
		}
		break;
	}
	RefreshVolume(vol);
}

ze_internal void TickWorldEdit(f32 delta, Vec2 mouseWorldPos, frameInt frameNumber)
{
	Point2 g = PosToGrid(mouseWorldPos);
	WorldVolume* vol = NULL;
	i32 bModifierOn = g_engine.input.GetActionValue(ED_MODIFIER_1) != 0;
	i32 bDelete = g_engine.input.GetActionValue(ED_DELETE) != 0;
	if (g_selectedVolumeId != 0)
	{
		if (g_engine.input.HasActionToggledOn(ED_MOUSE_2, frameNumber))
		{
			SetSelectedVolume(0);
			vol = NULL;
		}
		else
		{
			vol = (WorldVolume*)g_volumes.GetById(g_selectedVolumeId);
		}
	}

	// select or add
	if (vol == NULL && g_engine.input.HasActionToggledOn(ED_MOUSE_1, frameNumber))
	{
		vol = FindVolumeAt(g);
		if (vol == NULL)
		{
			AddVolumeAt(g);
			return;
		}
		else
		{
			SetSelectedVolume(vol->id);
		}
	}

	// cancel drag if modifier was released
	if (g_dragCorner != DRAG_CORNER_NONE && !bModifierOn)
	{
		g_dragCorner = DRAG_CORNER_NONE;
	}

	if (vol != NULL && bDelete)
	{
		SetSelectedVolume(0);
		RemoveVolume(vol->id);
		return;
	}

	// check for drag
	if (vol != NULL && bModifierOn)
	{
		if (g_dragCorner == DRAG_CORNER_NONE)
		{
			if (g_engine.input.HasActionToggledOn(ED_MOUSE_1, frameNumber))
			{
				g_dragCorner = SelectDragCorner(mouseWorldPos, g, vol);
			}
		}
		else if (g_engine.input.GetActionValue(ED_MOUSE_1) != 0)
		{
			MouseDragVolume(mouseWorldPos, g, vol);
		}
		else
		{
			g_dragCorner = DRAG_CORNER_NONE;
		}
	}
}

////////////////////////////////////////////////////////////////////
// commands
////////////////////////////////////////////////////////////////////
ze_internal void Ed_SaveMap(const char* fileName)
{
	RNGPRINT("Editor - save map %s\n", fileName);
	// alloc staging buffer
	zeSize capacity = MegaBytes(1);
	char* buf = (char*)g_engine.system.Malloc(capacity);
	ZE_SET_ZERO(buf, capacity);
	char* write = buf;
	char* end = buf + capacity;

	write += sprintf_s(write, end - write, "aabbs\n");
	i32 numVolumes = g_volumes.Count();
	for (i32 i = 0; i < numVolumes; ++i)
	{
		WorldVolume* vol = (WorldVolume*)g_volumes.GetByIndex(i);
		write += sprintf_s(write, end - write, "%d %d %d %d %d %d\n",
			vol->id, 0, vol->gridMin.x, vol->gridMin.y, vol->gridMax.x, vol->gridMax.y);
	}
	RNGPRINT("-- Save ASCII (%lld chars)--\n", end - write);
	RNGPRINT(buf);
	RNGPRINT("\n");
	g_engine.io.WriteFile(fileName, buf, write - buf);
	g_engine.system.Free(buf);
}

ze_internal void Ed_LoadMap(const char* fileName)
{
	RNGPRINT("Editor - load map %s\n", fileName);
}

ZCMD_CALLBACK(Ed_ExecCommand)
{
	if (numTokens == 1)
	{
		RNGPRINT("Editor command help\n");
		RNGPRINT("Ed save <fileName>\n");
		RNGPRINT("Ed load <fileName>\n");
		return;
	}
	char* subCommand = tokens[1];
	if (numTokens >= 2)
	{
		char* fileName = "temp.txt";
		if (numTokens >= 3)
		{
			fileName = tokens[2];
		}
		
		if (ZStr_Equal(subCommand, "save"))
		{
			Ed_SaveMap(fileName);
		}
		else if (ZStr_Equal(subCommand, "load"))
		{
			Ed_LoadMap(fileName);
		}
		else
		{
			RNGPRINT("Unrecognised option \"%s\"\n", subCommand);
		}
	}
}

////////////////////////////////////////////////////////////////////
// lifecycle
////////////////////////////////////////////////////////////////////

ze_external void Ed_Enable()
{
    u32 flags = g_engine.scenes.GetSceneFlags(g_edScene);
    flags &= ~ZSCENE_FLAG_NO_DRAW;
    g_engine.scenes.SetSceneFlags(g_edScene, flags);
}

ze_external void Ed_Disable()
{
    u32 flags = g_engine.scenes.GetSceneFlags(g_edScene);
    flags |= ZSCENE_FLAG_NO_DRAW;
    g_engine.scenes.SetSceneFlags(g_edScene, flags);
}

ze_internal void Ed_UpdateDebugText()
{
	g_debugText.Clear(NO);
	char* write = (char*)g_debugText.cursor;
	char* end = (char*)g_debugText.End();

	Point2 mouseGrid = PosToGrid(g_mouseWorldPos);
	write += sprintf_s(
		write,
		end - write,
		"Cursor pos: %.3f, %.3f\n. Grid: %d, %d\nDrag Corner %d\n",
		g_mouseWorldPos.x, g_mouseWorldPos.y, mouseGrid.x, mouseGrid.y,
		g_dragCorner
	);
	if (g_selectedVolumeId != 0)
	{
		WorldVolume* vol = (WorldVolume*)g_volumes.GetById(g_selectedVolumeId);
		Vec2 pos = GetVolumeCentre(vol);
		Point2 size = GetVolumeSize(vol);
		write += sprintf_s(
			write,
			end - write,
			"Selected volume: %d: %d, %d to %d, %d\nCentre: %.3f, %.3f. size %d, %d\nFracX/Y: %.3f, %.3f\n",
			g_selectedVolumeId,
			vol->gridMin.x, vol->gridMin.y,
			vol->gridMax.x, vol->gridMax.y,
			pos.x, pos.y,
			size.x, size.y,
			vol->fracX, vol->fracY
		);
	}
}

ze_internal EdInput GetEdInput()
{
	EdInput input = {};
	
	input.moveUp = g_engine.input.GetActionValue(ED_MOVE_UP) > 0;
	input.moveDown = g_engine.input.GetActionValue(ED_MOVE_DOWN) > 0;
	input.moveLeft = g_engine.input.GetActionValue(ED_MOVE_LEFT) > 0;
	input.moveRight = g_engine.input.GetActionValue(ED_MOVE_RIGHT) > 0;

	input.mouse1 = g_engine.input.GetActionValue(ED_MOUSE_1) > 0;
	input.mouse2 = g_engine.input.GetActionValue(ED_MOUSE_2) > 0;

	input.modifier1 = g_engine.input.GetActionValue(ED_MODIFIER_1) > 0;
	input.del = g_engine.input.GetActionValue(ED_DELETE) > 0;

	input.mode1 = g_engine.input.GetActionValue(ED_SOLID_GEOMETRY_MODE) > 0;
	input.mode2 = g_engine.input.GetActionValue(ED_LINE_GEOMETRY_MODE) > 0;
	input.mode3 = g_engine.input.GetActionValue(ED_ENTITY_MODE) > 0;

	return input;
}

ze_external void Ed_Tick(ZEFrameTimeInfo timing)
{
	// read input
	g_tick.lastInput = g_tick.input;
	g_tick.input = GetEdInput();
	g_tick.delta = (f32)timing.interval;
	g_tick.mouseScreenPos = App_GetCursorScreenPos();
	g_tick.frameNumber = timing.frameNumber;

	// move camera
	f32 camSpeed = 15.f;
	f32 camStep = camSpeed * (f32)timing.interval;
	Transform camera = g_engine.scenes.GetCamera(g_edScene);
	if (g_engine.input.GetActionValue(ED_MOVE_UP) > 0) { camera.pos.y += camStep; }
	if (g_engine.input.GetActionValue(ED_MOVE_DOWN) > 0) { camera.pos.y -= camStep; }
	if (g_engine.input.GetActionValue(ED_MOVE_LEFT) > 0) { camera.pos.x -= camStep; }
	if (g_engine.input.GetActionValue(ED_MOVE_RIGHT) > 0) { camera.pos.x += camStep; }
	g_engine.scenes.SetCamera(g_edScene, camera);

	// update cursor
	g_mouseWorldPos = App_GetCursorScreenPos();
	g_mouseWorldPos.x += camera.pos.x;
	g_mouseWorldPos.y += camera.pos.y;

	ZRDrawObj* obj = g_engine.scenes.GetObject(g_edScene, g_pointCursor);
	obj->t.pos.x = g_mouseWorldPos.x;
	obj->t.pos.y = g_mouseWorldPos.y;
	
	/*
	#define ED_SOLID_GEOMETRY_MODE "ed_solid_geometry_mode"
	#define ED_LINE_GEOMETRY_MODE "ed_line_geometry_mode"
	#define ED_ENTITY_MODE "ed_entity_mode"
	
	g_engine.input.HasActionToggledOn(ED_MOUSE_1, frameNumber)
	*/
	if (g_engine.input.HasActionToggledOn(ED_SOLID_GEOMETRY_MODE, timing.frameNumber))
	{
		g_editMode = 1;
	}
	else if (g_engine.input.HasActionToggledOn(ED_LINE_GEOMETRY_MODE, timing.frameNumber))
	{
		g_editMode = 2;
	}
	else if (g_engine.input.HasActionToggledOn(ED_ENTITY_MODE, timing.frameNumber))
	{
		g_editMode = 3;
	}
	
	

	TickWorldEdit((f32)timing.interval, g_mouseWorldPos, timing.frameNumber);
	Ed_UpdateDebugText();
	g_volumes.Truncate();
}

ze_internal void BuildGrid()
{
	i32 gridLines = 24;
	i32 gridVertCount = gridLines * gridLines * 3;
	ZRMeshAsset* grid = g_engine.assets.AllocEmptyMesh("grid", gridVertCount);
	const f32 lineSize = 0.02f;

	// vertical
	Vec3 bl = { -lineSize, -8, 0 };
	Vec3 br = { lineSize, -8, 0 }; 
	Vec3 tl = { -lineSize, 8, 0 }; 
	Vec3 tr = { lineSize, 8, 0 };
	Vec3 n = { 0, 0, -1 };
	Vec3 origin = { -12.f, 0.f };
	for (i32 i = 0; i < gridLines; ++i)
	{
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(br, origin), Vec3_Add(tr, origin), {}, {}, {}, n, n, n);
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(tr, origin), Vec3_Add(tl, origin), {}, {}, {}, n, n, n);
		origin.x += 1.f;
	}
	// horizontal
	bl = { -12, -lineSize, 0 };
	br = { 12, -lineSize, 0 }; 
	tl = { -12, lineSize, 0 }; 
	tr = { 12, lineSize, 0 };
	origin = { 0.f, -12.f };
	for (i32 i = 0; i < gridLines; ++i)
	{
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(br, origin), Vec3_Add(tr, origin), {}, {}, {}, n, n, n);
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(tr, origin), Vec3_Add(tl, origin), {}, {}, {}, n, n, n);
		origin.y += 1.f;
	}
	
	ZRDrawObj* gridObj = g_engine.scenes.AddObject(g_edScene);
	i32 matId = g_engine.assets.GetMaterialByName(FALLBACK_MATERIAL_NAME)->header.id;
	gridObj->data.SetAsMesh(grid->header.id, matId);
	RNGPRINT("Grid draw obj Id %d, mat Id %d\n", gridObj->id, matId);
}

ze_external char* Ed_GetDebugText()
{
	return (char*)g_debugText.start;
}

ze_external void Ed_Init(ZEngine engine)
{
    g_engine = engine;
    g_edScene = g_engine.scenes.AddScene(0, 4096, 0);
	g_engine.scenes.SetProjectionOrtho(g_edScene, 8);
	
	g_engine.textCommands.RegisterCommand("ed", "rng level editor commands. run with no params for help", Ed_ExecCommand);

	// register inputs
	g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, ED_MOVE_UP);
	g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, ED_MOVE_DOWN);
	g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, ED_MOVE_LEFT);
	g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, ED_MOVE_RIGHT);

	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_1, Z_INPUT_CODE_NULL, ED_MOUSE_1);
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_2, Z_INPUT_CODE_NULL, ED_MOUSE_2);

	g_engine.input.AddAction(Z_INPUT_CODE_LEFT_SHIFT, Z_INPUT_CODE_NULL, ED_MODIFIER_1);
	g_engine.input.AddAction(Z_INPUT_CODE_DELETE, Z_INPUT_CODE_NULL, ED_DELETE);
	
	/*
	#define ED_SOLID_GEOMETRY_MODE "ed_solid_geometry_mode"
	#define ED_LINE_GEOMETRY_MODE "ed_line_geometry_mode"
	#define ED_ENTITY_MODE "ed_entity_mode"
	*/
	g_engine.input.AddAction(Z_INPUT_CODE_1, Z_INPUT_CODE_NULL, ED_SOLID_GEOMETRY_MODE);
	g_engine.input.AddAction(Z_INPUT_CODE_2, Z_INPUT_CODE_NULL, ED_LINE_GEOMETRY_MODE);
	g_engine.input.AddAction(Z_INPUT_CODE_3, Z_INPUT_CODE_NULL, ED_ENTITY_MODE);
	
	// Create editor components
    BuildGrid();

	ZRDrawObj* pointCursor = g_engine.scenes.AddFullTextureQuad(
		g_edScene, FALLBACK_TEXTURE_WHITE, { 0.1f, 0.1f }, COLOUR_F32_RED);
	g_pointCursor = pointCursor->id;

	ZE_InitBlobStore(g_engine.system.Malloc, &g_volumes, 1024, sizeof(WorldVolume), 0);
	
	g_debugText = Buf_FromMalloc(g_engine.system.Malloc, MegaBytes(1));

	// finished
    Ed_Disable();
}
