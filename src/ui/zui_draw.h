#include "../../headers/common/ze_common_full.h"
#include "../../headers/zqf_renderer.h"
#include "zui.h"

/**
 * returns number of draw objects added
 */
internal i32 ZUI_WriteObjToScene(ZUIObject* uiObj, ZEBuffer* list, ZEBuffer* data)
{
    i32 numObjects = 0;
    ZRDrawObj* drawObj = NULL;
    
    // add bg
    #if 1
    drawObj = ZRDrawObj_InitInPlace(&list->cursor);
    numObjects++;
    drawObj->data.SetAsMesh(1, 0);
    drawObj->t.pos.x = uiObj->pos.x;
    drawObj->t.pos.y = uiObj->pos.y;
    drawObj->t.pos.z = 0.1f;// uiObj->depth;
    drawObj->t.scale =
    {
        ((f32)uiObj->radiusInChars.x * uiObj->charSize) * 2.f,
        ((f32)uiObj->radiusInChars.y * uiObj->charSize) * 2.f,
        1
    };
    #endif
    // add text
    if (uiObj->label != NULL)
    {
        Colour c = uiObj->offColour;
        if (uiObj->flags & ZUI_OBJ_FLAG_HOVER)
        {
            c = uiObj->onColour;
        }
        // create a temp text draw obj
        ZRDrawObj txtObj = {};
        txtObj.data.SetAsText(
            uiObj->label, -1, c, uiObj->bgColour, ZR_TEXT_ALIGNMENT_CENTRE);
        // push object toward camera slightly, away from background
        txtObj.t.pos.x = uiObj->pos.x;
        txtObj.t.pos.y = uiObj->pos.y;
        // TODO: Depth currently doesn't work for text!
        txtObj.t.pos.z -= 0.5f;
    
        // add to draw list
        ZR_WriteTextObj(&txtObj, list, data);
        numObjects++;
    }
    
    return numObjects;
}

extern "C" void ZUI_WriteScreenForRender(ZRViewFrame* frame, ZUIScreen* scr, ZEBuffer* list, ZEBuffer* data)
{
	if (scr->state == 0) { return; }
    ///////////////////////////////////////////////
    // Start a new scene
	i32 sceneCount = 0;
    ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
    sceneCount++;
    scene->params.objects = (ZRDrawObj*)list->cursor;
    scene->sentinel = ZR_SENTINEL;
    scene->params.projectionMode = ZR_PROJECTION_MODE_IDENTITY;
    Transform_SetToIdentity(&scene->params.camera);
    
    ///////////////////////////////////////////////
    // add objects
    for (i32 i = 0; i < scr->numObjects; ++i)
    {
        scene->params.numObjects += ZUI_WriteObjToScene(&scr->objects[i], list, data);
    }
    // Finish scene
    scene->params.numListBytes = list->cursor - (u8*)scene->params.objects;
    frame->numScenes += sceneCount;
    //return sceneCount;
}

#if 0
extern "C" i32 ZUI_WriteRenderTest(ZEBuffer* list, ZEBuffer* data)
{
    ///////////////////////////////////////////////
    // Start a new scene
	i32 sceneCount = 0;
    ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
    sceneCount++;
    scene->params.objects = (ZRDrawObj*)list->cursor;
    scene->sentinel = ZR_SENTINEL;
    scene->params.projectionMode = ZR_PROJECTION_MODE_IDENTITY;
    Transform_SetToIdentity(&scene->params.camera);
    
    ///////////////////////////////////////////////
    // add objects
    // ZUIObject* uiObj = &g_testObj;
    // ZRDrawObj* drawObj = NULL;
    scene->params.numObjects += ZUI_WriteObjToScene(&g_testObj, list, data);
    scene->params.numObjects += ZUI_WriteObjToScene(&g_testObj2, list, data);

    // Finish scene
    scene->params.numListBytes = list->cursor - (u8*)scene->params.objects;
	return sceneCount;
}

/**
 * Return scenes written
 */
extern "C" i32 ZUI_WriteRenderTest_2(ZEBuffer* list, ZEBuffer* data)
{
    ///////////////////////////////////////////////
    // Start a new scene
	i32 sceneCount = 0;
    ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
    sceneCount++;
    scene->sentinel = ZR_SENTINEL;
    scene->params.projectionMode = ZR_PROJECTION_MODE_IDENTITY;
    Transform_SetToIdentity(&scene->params.camera);

    scene->params.objects = (ZRDrawObj*)list->cursor;
    
    ///////////////////////////////////////////////
    // add objects

	//char* str = "The quick\nbrown fox jumped over\nthe lazy\ndogs.";
	char* str = "The quick brown fox.";
    // measure
    i32 len = ZStr_Len(str);
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
		strCursor, -1, COLOUR_WHITE, COLOUR_EMPTY, align);
	#endif
    // Finish scene
    scene->params.numListBytes = list->cursor - (u8*)scene->params.objects;
	
	return sceneCount;
}
#endif
