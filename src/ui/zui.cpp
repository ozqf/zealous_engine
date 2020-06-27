#ifndef ZUI_CPP
#define ZUI_CPP

#include "../ze_common/ze_common_full.h"
#include "../zqf_renderer.h"

extern "C" void ZUI()
{
	//
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
    ZRDrawObj* uiObj;

    uiObj = ZRDrawObj_InitInPlace(&list->cursor);
    scene->params.numObjects++;
    uiObj->data.SetAsMesh(1, 0);
    uiObj->t.pos.z = 0.9f;

    uiObj = ZRDrawObj_InitInPlace(&list->cursor);
    scene->params.numObjects++;
	// copy string to data buffer
    char* str = "Foobar";
    // measure
    i32 len = ZE_StrLen(str);
    // grab start of write
    char* strCursor = (char*)data->cursor;
    // copy
    data->cursor += ZE_Copy(data->cursor, str, len);
    // set object
    uiObj->data.SetAsText(strCursor);
	
    // Finish scene
    scene->params.numDataBytes = list->cursor - (u8*)scene->params.objects;
	
	return sceneCount;
}

#endif // ZUI_CPP