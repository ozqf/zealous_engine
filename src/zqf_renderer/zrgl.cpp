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
#include "zrgl_forward_draw.h"
#include "zrgl_forward_main.h"
// deferred
#include "zrgl_deferred_draw.h"
#include "zrgl_gbuffer.h"
#include "zrgl_deferred_main.h"

#include "zrgl_init.h"


extern "C" ZRPerformanceStats ZRGL_DrawFrame(
	ZEByteBuffer* drawList,
    ZEByteBuffer* drawData,
    ScreenInfo scrInfo)
{
	/*
extern "C" ZRPerformanceStats ZRImpl_DrawFrameForward(
    ZEByteBuffer* drawList,
    ZEByteBuffer* drawData,
    ScreenInfo scrInfo);

extern "C" ZRPerformanceStats ZRImpl_DrawFrameDeferred(
    ZEByteBuffer* drawList,
    ZEByteBuffer* drawData,
    ScreenInfo scrInfo);
	*/

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
	// iterate scenes, generating draw groups.
	u8* groupsCursor = cursor;
	for (i32 i = 0; i < header->numScenes; ++i)
	{
		ZRSceneFrame* scene = (ZRSceneFrame*)groupsCursor;
		ZE_ASSERT(scene->sentinel == ZR_SENTINEL, "Iterate scenes desync");
    	groupsCursor += sizeof(ZRSceneFrame) + scene->params.numDataBytes;
		if (g_verboseFrame)
		{
			printf("Scene %d - %d objects, %dKB\n",
				i, scene->params.numObjects, scene->params.numDataBytes / 1024);
		}
		//ZRDrawGroup* group = ZR_BuildDrawGroups(scene->params.numDataBytes,)
	}

    /////////////////////////////////////////////////////////////
    // Draw first scene - deferred if that is set
	
    ZRSceneFrame* firstScene = (ZRSceneFrame*)cursor;
    cursor += sizeof(ZRSceneFrame) + firstScene->params.numDataBytes;
    ZRGroupingStats gBufStats = ZR_DrawSceneDeferred(firstScene, &g_scratch, scrInfo);

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
    
    f32 screenSpaceHeight = 2;
    f32 numLinesInScreen = 64;
    ZRDrawCmd_Text txtCmd = {};
    txtCmd.origin = { -1, 1 }; // screen topleft
    txtCmd.numChars = written;// strlen(testText);
    txtCmd.charSize = screenSpaceHeight / numLinesInScreen;
    txtCmd.aspectRatio = scrInfo.aspectRatio;
    txtCmd.offsetToString = 0; // TOOD: Remove
    txtCmd.alignmentMode = 0;
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