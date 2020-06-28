#ifndef ZRGL_CPP
#define ZRGL_CPP

#include "../ze_common/ze_common.h"

// Performs non-opengl related logic
#include "zr_groups.h"

// third party
#include "../../lib/glad/glad.h"
#include "../../lib/glfw3_vc2015/glfw3.h"

#include "zrgl_internal.h"

// opengl specific implementations
#include "zrgl_buffers.h"
#include "zrgl_shaders.h"
#include "zrgl_prefabs.h"
#include "zrgl_shadows.h"
#include "zrgl_upload.h"
// Forward
#include "zrgl_text.h"
#include "zrgl_forward_draw.h"
#include "zrgl_forward_main.h"
// deferred
//#include "zrgl_deferred_draw.h"
#include "zrgl_gbuffer.h"
#include "zrgl_deferred_main.h"

#include "zrgl_init.h"


extern "C" ZRPerformanceStats ZRGL_DrawFrame(
	ZEByteBuffer* drawList,
    ZEByteBuffer* drawData,
    ScreenInfo scrInfo)
{
	/////////////////////////////////////////////////////////////
	// Setup for draw
	/////////////////////////////////////////////////////////////
	ZRPerformanceStats stats = {};
    if (Buf_IsValid(drawList) == NO) { return stats; }
    if (Buf_IsValid(drawData) == NO) { return stats; }
	// This will almost always happen
	// whilst the app thread starts up
	i32 listBytes = drawList->Written();
	if (listBytes == 0)
	{
		return stats;
	}

    if (g_bDrawLocked == YES)
    {
        ILLEGAL_CODE_PATH
    }
    g_bDrawLocked = YES;

    stats.listBytes = drawList->Written();
    stats.numDataBytes = drawData->Written();

    // Reset frame scratch memory cursor
    g_scratch.Clear(NO);
    // allocate some space in scratch for debug string
    ZEByteBuffer debugStr = Buf_SubBuffer(&g_scratch, KiloBytes(4));
    if (debugStr.capacity == 0)
    {
        printf("Failed to allocate space for debug str\n");
    }

    // Reset data texture
    g_dataTex2D.cursor = 0;

    u8* cursor = drawList->start;
    u8* end = drawList->cursor;

	/////////////////////////////////////////////////////////////
    // Clear
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR
    glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERR
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERR

	/////////////////////////////////////////////////////////////
	// Start - preprocess
	/////////////////////////////////////////////////////////////
	ZRViewFrame* header = (ZRViewFrame*)cursor;
    
	if (header->bVerbose == YES)
    {
        printf("ZRGL - Verbose frame!\n");
        g_verboseFrame = YES;
    }
    ZE_ASSERT(header->sentinel == ZR_SENTINEL, "Sentinel check failed")
    cursor += sizeof(ZRViewFrame);
    u8* scenesStart = cursor;

	/////////////////////////////////////////////////////////////
	// iterate scenes,  generating draw groups and preparing data
	ZRGroupingStats groupStats = {};
	u8* groupsCursor = cursor;
	for (i32 i = 0; i < header->numScenes; ++i)
	{
		ZRSceneFrame* scene = (ZRSceneFrame*)groupsCursor;
		ZE_ASSERT(scene->sentinel == ZR_SENTINEL, "Iterate scenes desync");
    	groupsCursor += sizeof(ZRSceneFrame) + scene->params.numDataBytes;

		ZRGL_SetupProjection(
			&scene->drawTime.projection,
			scene->params.projectionMode,
			scrInfo.aspectRatio
		);

		// Process scen into groups
		ZRSceneView* view = ZR_BuildDrawGroups(
			scene->params.objects, scene->params.numObjects, &g_scratch, &groupStats);
		scene->drawTime.view = view;
		
		// Write group batches to data texture
		i32 dataCursorStart = g_dataTex2D.cursor;
		ZR_WriteGroupsToTextureByIndex(
			scene->params.objects,
			scene->params.numObjects,
			&scene->params.camera,
			view,
			&g_dataTex2D);
		i32 dataCursorEnd = g_dataTex2D.cursor;

		////////////////////////////////////////
		// Debug spam
		if (g_verboseFrame)
		{
			printf("--- Scene %d - %d objects, %dKB ---\n",
				i, scene->params.numObjects, scene->params.numDataBytes / 1024);
			printf("\tGroups %d, lights %d\n",
				view->numGroups,
				view->numLights);
			for (i32 j = 0; j < view->numGroups; ++j)
			{
				printf("\tGroup %d. type %d, objects %d, batchable %d\n",
					j,
					view->groups[j]->data.type,
					view->groups[j]->numItems,
					view->groups[j]->bBatchable);
			}
			printf("%d data texture pixels\n", dataCursorEnd - dataCursorStart);
			printf("\n");
		}

	}

    /////////////////////////////////////////////////////////////
	// Preprocess finish
    // --- All batch data written by this point! ---
    f64 uploadStart = g_platform.QueryClock();
    #if 1
    ZR_UploadDataTexture();
    #endif
    f64 uploadEnd = g_platform.QueryClock();

    /////////////////////////////////////////////////////////////
	// Draw
	/////////////////////////////////////////////////////////////
	ZRGroupingStats gBufStats = {};
	for (i32 i = 0; i < header->numScenes; ++i)
	{
		// Draw first scene - deferred if that is set
		ZRSceneFrame* scene = (ZRSceneFrame*)cursor;
    	cursor += sizeof(ZRSceneFrame) + scene->params.numDataBytes;

		if (scene->params.bDeferred)
		{
			gBufStats = ZR_DrawSceneDeferred(scene, &g_scratch, scrInfo);

    		// Draw debug cack
    		#if 0
    		ZRGL_DrawGBufferDebugQuads(scrInfo.aspectRatio);
    		#endif
		}
		else
		{
			ZRSceneView* v = scene->drawTime.view;
			for (i32 j = 0; j < v->numGroups; ++j)
			{
				ZRDrawGroup* group = v->groups[j];
				ZR_DrawGroupForward(
					&scene->params.camera,
					scene->params.objects,
					scene->params.numObjects,
					&scene->drawTime.projection,
					group,
					&scrInfo,
					&stats);
			}
		}
	}

    //ZRSceneFrame* firstScene = (ZRSceneFrame*)cursor;
    //cursor += sizeof(ZRSceneFrame) + firstScene->params.numDataBytes;

	//ZR_DrawMeshGroupBatched()

    /////////////////////////////////////////
    // Draw debug text
    /////////////////////////////////////////
    
    // allocate space in scratch for debug string
    #if 1
    i32 written = sprintf_s((char*)debugStr.cursor, debugStr.Space(),
        "Prebuild: %.3fMS\nObj List %dKB\nObj Data %dKB\nGBuffer Fill %.3fMS\nGBuffer Light %.3fMS\nNum lights: %d\nSwapMS %.3f\nTotalMS %.3f\n",
        header->prebuildTime * 1000,
        drawList->Written() / 1024,
        drawData->Written() / 1024,
        gBufStats.gBufferFillMS,
        gBufStats.gBufferLightMS,
        gBufStats.numLights,
        g_platformSwapMS * 1000,
        g_platformFrameMS * 1000);
    debugStr.cursor += written;
    
    ZRDrawCmd_Text txtCmd = {};
    txtCmd.origin = { -1, 1 }; // screen topleft
    txtCmd.numChars = written;// strlen(testText);
    txtCmd.charSize = ZR_SCREEN_SPACE_HEIGHT / ZR_TEXT_SCREEN_LINE_COUNT;
    txtCmd.aspectRatio = scrInfo.aspectRatio;
    txtCmd.offsetToString = 0; // TOOD: Remove
    txtCmd.alignmentMode = ZR_TEXT_ALIGNMENT_TOP_LEFT;
    M4x4_CREATE(textProjection);
    //printf("Draw %d debug chars\n", txtCmd.numChars);
    ZR_ExecuteTextDraw(&txtCmd, &textProjection, (char*)debugStr.start, &stats);
    #endif

	
    /////////////////////////////////////////////////////////////
    // Done. Gather stats
	/////////////////////////////////////////////////////////////
    g_bDrawLocked = NO;
	g_verboseFrame = NO;
    
    // f64 drawEnd = g_platform.QueryClock();
    // stats.prepareTime = (prepareEnd - prepareStart) * 1000;
	// stats.uploadTime = (uploadEnd - uploadStart) * 1000;
	// stats.drawTime = (drawEnd - drawStart) * 1000;
	// stats.total = stats.prepareTime + stats.uploadTime + stats.drawTime;
	// i32 texIndex = g_dataTex2D.cursor;
	// i32 totalPixels = g_dataTex2D.width * g_dataTex2D.height;
    // stats.dataTexPercentUsed = ((f32)texIndex / (f32)totalPixels) * 100.f;
    return stats;
}


#endif // ZRGL_CPP