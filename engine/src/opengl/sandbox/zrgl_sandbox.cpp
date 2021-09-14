
//////////////////////////////////////////////////
// This module should be isolated and purely for
// debugging/tests etc
//////////////////////////////////////////////////

#include "../ze_opengl_internal.h"

#include "shaders/sandbox_shaders.h"

#include "draw_screen_quad_test.h"
#include "draw_screen_cube_test.h"
#include "draw_world_cube_test.h"
#include "draw_world_sprites_test.h"
#include "draw_quad_batch_test.h"
#include "draw_sprite_batch_test.h"
#include "draw_sprite_batch_test_2.h"

ze_external void ZRGL_Debug_Init()
{

}

ze_external void ZRGL_SandboxRunTest(i32 mode)
{
	switch (mode)
    {
        case 1:
        OpenglTest_DrawScreenSpaceQuad();
        break;
        case 2:
        ZRGL_Debug_DrawCubeTest();
        break;
        case 3:
        ZRGL_Debug_DrawWorldCubeTest();
        break;
        case 4:
        ZRGL_Debug_DrawWorldSprites();
        break;
        case 5:
        ZRSandbox_DrawQuadBatch();
        break;
        case 6:
        ZRSandbox_DrawSpriteBatch();
        break;
		case 7:
		ZRSandbox_DrawSpriteBatch_2();
		break;

        default:
        ZRGL_Debug_DrawWorldCubeTest();
        break;
    }
}
