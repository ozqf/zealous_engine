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
#include "zrgl_sprites.h"
// Forward
#include "zrgl_text.h"
#include "zrgl_forward_draw.h"
#include "zrgl_forward_main.h"
// deferred
//#include "zrgl_deferred_draw.h"
#include "zrgl_gbuffer.h"
#include "zrgl_deferred_main.h"

#include "zrgl_init.h"

extern "C" i32 ZRGL_ExecTextCommand(
	const char* str, const char** tokens, const i32 numTokens)
{
	if (ZE_CompareStrings(tokens[0], "HELP") == 0)
	{
		printf("RSTATS - toggle display render info\n");
		printf("LIGHTMODE modeInt - set 3D scene lighting mode\n");
		printf("GBUFFER - toggle gbuffer mode\n");
		return NO;
	}
	if (ZE_CompareStrings(tokens[0], "LIGHTMODE") == 0)
	{
		printf("ZR - change light mode\n");
		if (numTokens == 2)
		{
			i32 mode = ZE_AsciToInt32(tokens[1]);
			printf("\t mode %d\n", mode);
			g_lightingMode = mode;
		}
		return YES;
	}
	if (ZE_CompareStrings(tokens[0], "GBUFFER") == 0)
	{
		g_debugFlags ^= ZRGL_DEBUG_BIT_SHOWGBUFFER;
		return YES;
	}
	if (ZE_CompareStrings(tokens[0], "RSTATS") == 0)
	{
		g_debugFlags ^= ZRGL_DEBUG_BIT_SHOWSTATS;
		return YES;
	}
	return NO;
}

internal void ZRGL_ErrorDumpFrameData(ZEBuffer* drawList,
    ZEBuffer* drawData,
    ScreenInfo scrInfo)
{
	u8* cursor = drawList->start;
	printf("=== Frame data ===\n");
	if (drawList->Written() < sizeof(ZRViewFrame))
	{
		printf("- No space for a frame struct found\n");
		return;
	}
	ZRViewFrame* header = (ZRViewFrame*)cursor;
	if (header->sentinel != ZR_SENTINEL)
	{
		printf("ZRViewFrame sentinel expected %d got %d\n",
			ZR_SENTINEL, header->sentinel);
		return;
	}
	cursor += sizeof(ZRViewFrame);
	printf("%d scenes\n", header->numScenes);
	// iterate scenes
    u8* scenesStart = cursor;
	for (i32 i = 0; i < header->numScenes; ++i)
	{
		ZRSceneFrame* scene = (ZRSceneFrame*)cursor;
		if (scene->sentinel != ZR_SENTINEL)
		{
			printf("Desync scene %d: expected scene sentinel %d got %d\n",
				i, ZR_SENTINEL, scene->sentinel);
			return;
		}
		printf("Scene %d: %d objs, %d bytes\n",
			i, scene->params.numObjects, scene->params.numListBytes);
    	cursor += sizeof(ZRSceneFrame) + scene->params.numListBytes;
	}
}

extern "C" ZRPerformanceStats ZRGL_DrawFrame(
	ZEBuffer* drawList,
    ZEBuffer* drawData,
    ScreenInfo scrInfo)
{
	g_framesRendered++;

	if (AssetDb()->bDirty)
	{
		//printf("ZRGL - db is dirty - checking for uploads\n");
		AssetDb()->bDirty = NO;
		ZRGL_CheckForUploads(NO);
	}
	// Update time
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
    stats.numListBytes = drawData->Written();

    // Reset frame scratch memory cursor
    g_scratch.Clear(NO);
    // allocate some space in scratch for debug string
    ZEBuffer debugStr = Buf_SubBuffer(&g_scratch, KiloBytes(4));
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
	// Update timing info for interpolation
	/////////////////////////////////////////////////////////////
	f64 diff = 0;
	if (g_lastFrameNumber < header->frameNumber)
	{
		// New frame
		g_lastFrameNumber = header->frameNumber;
		g_lastTimestamp = g_platform.QueryClock();
		g_interpolate = 0;
		diff = 0;
	}
	else
	{
		diff = g_platform.QueryClock() - g_lastTimestamp;
		g_interpolate = (f32)diff / (1.f / 60.f);
		if (g_interpolate > 1) { g_interpolate = 1; }
		if (g_interpolate < 0) { g_interpolate = 0; }
		//printf("ZRGL diff - %.3f interpolate %.3f\n", diff, g_interpolate);
	}

	/////////////////////////////////////////////////////////////
	// iterate scenes,  generating draw groups and preparing data
	f64 preprocessStart = g_platform.QueryClock();
	ZRGroupingStats groupStats = {};
	u8* groupsCursor = cursor;
	for (i32 i = 0; i < header->numScenes; ++i)
	{
		ZRSceneFrame* scene = (ZRSceneFrame*)groupsCursor;
		if (scene->sentinel != ZR_SENTINEL)
		{
			// fatal error - scene/object read desync
			ZE_BUILD_STRING(errStr, 512, "%s: %d - Iterate scenes desync on scene %d\n",
				__FILE__, __LINE__, i);
			printf("%s\n", errStr);
			ZRGL_ErrorDumpFrameData(drawList, drawData, scrInfo);
			ZE_Fatal(errStr);
		}
		//ZE_ASSERT(scene->sentinel == ZR_SENTINEL, "Iterate scenes desync");
    	groupsCursor += sizeof(ZRSceneFrame) + scene->params.numListBytes;
		if (scene->params.numObjects > 0 && scene->params.numListBytes <= 0)
		{
			ZE_ASSERT(NO, "Scene claims to have objects but no data bytes.")
		}

		ZRGL_SetupProjection(
			&scene->drawTime.projection,
			scene->params.projectionMode,
			scrInfo.aspectRatio
		);

		// Process scen into groups
		ZRSceneView* view = ZR_BuildDrawGroups(
			scene->params.objects, scene->params.numObjects, &g_scratch, &groupStats);
		scene->drawTime.view = view;
		stats.numGroups += view->numGroups;
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
				i, scene->params.numObjects, scene->params.numListBytes / 1024);
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
	f64 preprocessEnd = g_platform.QueryClock();

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
    	cursor += sizeof(ZRSceneFrame) + scene->params.numListBytes;
		// ignore empty scenes
		if (scene->params.numObjects == 0) { continue; }

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
		///////////////////////////////////
		// Clear depth buffer for next scene
		glClear(GL_DEPTH_BUFFER_BIT);
		CHECK_GL_ERR
	}

    //ZRSceneFrame* firstScene = (ZRSceneFrame*)cursor;
    //cursor += sizeof(ZRSceneFrame) + firstScene->params.numListBytes;

	//ZR_DrawMeshGroupBatched()

	if (g_debugFlags & ZRGL_DEBUG_BIT_SHOWGBUFFER)
	{
		ZRGL_DrawGBufferDebugQuads(scrInfo.aspectRatio);
	}

    /////////////////////////////////////////
    // Draw debug text
    /////////////////////////////////////////
    
    // allocate space in scratch for debug string
	if (g_debugFlags & ZRGL_DEBUG_BIT_SHOWSTATS)
	{
		f64 preprocessTime = preprocessEnd - preprocessStart;
    	i32 written = sprintf_s((char*)debugStr.cursor, debugStr.Space(),
    	    "Timestamp: %.3f\nTime diff: %.3f\nInterpolate: %.3f\nBuild drawlist: %.3fMS\nObj List %dKB\nObj Data %dKB\nNum scenes %d\nNum groups %d\nNum lights: %d\nPreprocessMS %.3f\nSwapMS %.3f\nTotalMS %.3f\nApp frames %d\nRenderer frames %d\n",
			header->timestamp,
			diff,
			g_interpolate,
    	    header->prebuildTime * 1000,
    	    drawList->Written() / 1024,
    	    drawData->Written() / 1024,
    	    header->numScenes,
			stats.numGroups,
    	    gBufStats.numLights,
			preprocessTime * 1000,
    	    g_platformSwapMS * 1000,
    	    g_platformFrameMS * 1000,
			header->frameNumber,
			g_framesRendered);
    	debugStr.cursor += written;
    
    	ZRDrawCmd_Text txtCmd = {};
    	txtCmd.origin = { -1, 1 }; // screen topleft
    	txtCmd.numChars = written;// strlen(testText);
    	txtCmd.charSize = ZR_SCREEN_SPACE_HEIGHT / ZR_TEXT_SCREEN_LINE_COUNT;
    	txtCmd.aspectRatio = scrInfo.aspectRatio;
    	txtCmd.offsetToString = 0; // TOOD: Remove
    	txtCmd.alignmentMode = ZR_TEXT_ALIGNMENT_TOP_LEFT;
		txtCmd.fontColour = COLOUR_GREEN;
		txtCmd.bgColour = COLOUR_BLACK;
    	M4x4_CREATE(textProjection);
    	//printf("Draw %d debug chars\n", txtCmd.numChars);
    	ZR_ExecuteTextDraw(&txtCmd, &textProjection, (char*)debugStr.start, &stats);
	}
	
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