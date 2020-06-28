#ifndef ZUI_CPP
#define ZUI_CPP

#include "../ze_common/ze_common_full.h"
#include "../zqf_renderer.h"
#include "zui.h"

static ZRAssetDB* g_db = NULL;

extern "C" void ZUI_Init(ZRAssetDB* db)
{
	g_db = db;
}

/**
 * Return scenes written
 */
extern "C" i32 ZUI_WriteRenderTest(ZEByteBuffer* list, ZEByteBuffer* data)
{
	i32 sceneCount = 0;
    ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
    sceneCount++;
    scene->sentinel = ZR_SENTINEL;
    scene->params.projectionMode = ZR_PROJECTION_MODE_IDENTITY;
    Transform_SetToIdentity(&scene->params.camera);

    scene->params.objects = (ZRDrawObj*)list->cursor;

    // add objects
	//char* str = "The quick\nbrown fox jumped over\nthe lazy\ndogs.";
	char* str = "The quick brown fox.";
    // measure
    i32 len = ZE_StrLen(str);
    ZRDrawObj* uiObj;

    uiObj = ZRDrawObj_InitInPlace(&list->cursor);
    scene->params.numObjects++;
    uiObj->data.SetAsMesh(1, 0);
    uiObj->t.pos.z = 0.9f;
	f32 charSize = ZR_CharScreenSizeDefault();
	uiObj->t.scale = { charSize * len, charSize, 1 };
	#if 1
    uiObj = ZRDrawObj_InitInPlace(&list->cursor);
    scene->params.numObjects++;
	// copy string to data buffer
    // grab start of write
    char* strCursor = (char*)data->cursor;
    // copy
    data->cursor += ZE_Copy(data->cursor, str, len);
    // set object
	i32 align = ZR_TEXT_ALIGNMENT_CENTRE;
	// i32 align = ZR_TEXT_ALIGNMENT_TOP_RIGHT;
    uiObj->data.SetAsText(
		strCursor, -1, COLOUR_WHITE, align);
	#endif
    // Finish scene
    scene->params.numDataBytes = list->cursor - (u8*)scene->params.objects;
	
	return sceneCount;
}

#endif // ZUI_CPP