#include "../../headers/zqf_renderer.h"
#include "../../headers/zr_scene.h"
#include "../../headers/common/ze_blob_store.h"

struct ZRScene
{
	//ZRPlatform platform;

	// Tightly packed list of objects
	ZEBlobStore objects;
	i32 bSkybox;
	i32 bDeferred;
	i32 bDebug;
	i32 nextId;
	i32 numObjects;
	i32 maxObjects;

	i32 projectionMode;
	Transform camera;
};

ZRS_EXTERNAL ZRDrawObj* ZRS_AddObject(ZRScene* s)
{

}

ZRS_EXTERNAL void ZRS_RemoveObject(ZRScene *s)
{

}

ZRS_EXTERNAL void ZRS_WriteSceneForDraw(ZRViewFrame* frame)
{

}

ZRS_EXTERNAL ErrorCode ZRS_CreateScene(ZRScene** result, i32 capacity)
{

}
