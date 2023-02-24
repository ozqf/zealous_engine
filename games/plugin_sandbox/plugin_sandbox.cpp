#include "../../headers/zengine.h"

#include "../../plugins/zt_map_converter.h"

internal zeHandle g_gameScene = 0;
internal ZEngine g_engine;

internal Transform g_debugOrigin;
internal Transform g_debugCam;

internal void RunMapParseTest()
{
	// ZT_MapConvert("foo");
    printf("\n=== Test Map Converter ===\n");
    ZTMapOutput* output;
    ZT_MapConvertTest(&output);
	
	if (output == NULL)
	{
		printf("\tMap convert result is null!\n");
		return;
	}
	printf("Read %d verts from map result\n", output->numVerts);
	for (int i = 0; i < output->numVerts; ++i)
	{
		Vec3 pos = output->verts[i];
		Vec3_MulFPtr(&pos, 0.25f);
		ZRDrawObj* obj = g_engine.scenes.AddCube(g_gameScene, NULL);
		obj->t.pos = pos;
	}
}

internal void Init()
{
    // register a visual scene
    g_gameScene = g_engine.scenes.AddScene(0, 1024, 0);

	Transform_SetToIdentity(&g_debugCam);
	g_debugOrigin = g_engine.scenes.GetCamera(g_gameScene);

	// test plugins
    RunMapParseTest();
    printf("\n");
	
    // register inputs
    g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
    g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "move_forward");
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_backward");

    g_engine.input.AddAction(Z_INPUT_CODE_SPACE, Z_INPUT_CODE_NULL, "move_up");
    g_engine.input.AddAction(Z_INPUT_CODE_LEFT_SHIFT, Z_INPUT_CODE_NULL, "move_down");

    g_engine.input.AddAction(Z_INPUT_CODE_UP, Z_INPUT_CODE_NULL, "look_up");
    g_engine.input.AddAction(Z_INPUT_CODE_DOWN, Z_INPUT_CODE_NULL, "look_down");
    g_engine.input.AddAction(Z_INPUT_CODE_LEFT, Z_INPUT_CODE_NULL, "look_left");
    g_engine.input.AddAction(Z_INPUT_CODE_RIGHT, Z_INPUT_CODE_NULL, "look_right");

    g_engine.input.AddAction(Z_INPUT_CODE_Q, Z_INPUT_CODE_NULL, "roll_left");
    g_engine.input.AddAction(Z_INPUT_CODE_E, Z_INPUT_CODE_NULL, "roll_right");

    g_engine.input.AddAction(Z_INPUT_CODE_R, Z_INPUT_CODE_NULL, "reset");

}

internal void Shutdown()
{

}

internal void TickCamera(f32 delta)
{
	i32 bMoved = NO;
    Vec3 move = {};
    f32 speed = 15;
    if (g_engine.input.GetActionValue("reset"))
    {
        g_debugCam = g_debugOrigin;
        bMoved = YES;
    }

    if (g_engine.input.GetActionValue("move_left"))
    {
        move.x -= 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_right"))
    {
        move.x += 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_forward"))
    {
        move.z -= 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_backward"))
    {
        move.z += 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_down"))
    {
        move.y -= 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_up"))
    {
        move.y += 1;
        bMoved = YES;
    }
    
    Vec3 result = M3x3_Calculate3DMove(&g_debugCam.rotation, move);
    Vec3_MulFPtr(&result, speed * delta);
    Vec3_AddTo(&g_debugCam.pos, result);
    
    float rotRate = 90.f * DEG2RAD;
    if (g_engine.input.GetActionValue("look_up"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, rotRate * delta, 1, 0, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("look_down"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, -rotRate * delta, 1, 0, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("look_left"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, rotRate * delta, 0, 1, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("look_right"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, -rotRate * delta, 0, 1, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("roll_left"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, rotRate * delta, 0, 0, 1);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("roll_right"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, -rotRate * delta, 0, 0, 1);
        bMoved = YES;
    }

    if (bMoved)
    {
        // printf("Cam pos: %.3f, %.3f, %.3f\n",
        //     g_debugCam.pos.x, g_debugCam.pos.y, g_debugCam.pos.z);
        g_engine.scenes.SetCamera(g_gameScene, g_debugCam);
    }
}

internal void Tick(ZEFrameTimeInfo timing)
{
	TickCamera((f32)timing.interval);
}

Z_GAME_WINDOWS_LINK_FUNCTION
{
    g_engine = engineImport;
    gameExport->Init = Init;
    gameExport->Tick = Tick;
    gameExport->Shutdown = Shutdown;
    gameExport->sentinel = ZE_SENTINEL;
    return ZE_ERROR_NONE;
}
