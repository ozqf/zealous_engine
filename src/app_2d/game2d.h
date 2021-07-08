/*
Simple 2D game implementation
*/
#include "../../headers/common/ze_common_full.h"
#include "../../headers/zqf_renderer.h"

struct Ent
{
	i32 id;
	i32 type;
	// transform
	Vec3 pos;
	Vec2 scale;
	f32 radians;
	// movement
	Vec2 velocity;
	// display
	i32 meshId;
	i32 texId;
};

extern "C" void G2d_Init();
extern "C" void G2d_Tick(f32 delta);
extern "C" void G2d_Draw(ZRViewFrame *frame);
extern "C" void G2d_Shutdown();
