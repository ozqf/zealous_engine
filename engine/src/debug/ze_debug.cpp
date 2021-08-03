#include "../../internal_headers/zengine_internal.h"

internal zeHandle g_scene = 0;

ze_external zErrorCode ZDebug_Init_1()
{
	return ZE_ERROR_NONE;
}

ze_external zErrorCode ZDebug_Init_2()
{
	#if 1
	ZEngine engine = GetEngine();
    // find some assets to use on our objects
    i32 textureId = ZAssets_GetTexByName(
        FALLBACK_CHARSET_TEXTURE_NAME)->header.id;

	g_scene = ZScene_CreateScene(ZR_INTERNAL_SCENE_START_DEPTH, 16);

	// add an object to the scene
	ZRDrawObj *obj1 = ZScene_AddObject(g_scene);
	f32 size = 2.f / 64.f;
	// configure object
	obj1->data.SetAsText(
		"Debug\nScene", textureId, COLOUR_U32_GREEN, COLOUR_U32_EMPTY, 0);
	obj1->t.pos.x = -1;
	obj1->t.pos.y = 1;
	obj1->t.scale = { size, size, size };
	
	// set the camera and projection for the scene
	TRANSFORM_CREATE(camera)
	M4x4_CREATE(projection)
	engine.scenes.SetCamera(g_scene, camera);
	engine.scenes.SetProjection(g_scene, projection);
	#endif
	return ZE_ERROR_NONE;
}
