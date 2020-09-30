#ifndef GAME_DRAW_H
#define GAME_DRAW_H

/**
 * Public interface to client render module
 * > Initialises ZRViewFrame for draw lists
 * > Adds objects in the given Sim to the render list.
 */
#include "../../ze_common/ze_common.h"
//#include "../client/client.h"
#include "../../sim/sim.h"
#include "../../assetdb/zr_asset_db.h"

#define CL_DEBUG_FLAG_DRAW_LOCAL_SERVER (1 << 0)
#define CL_DEBUG_FLAG_NO_ENEMY_TICK (1 << 1)
#define CL_DEBUG_FLAG_NO_PLAYER_SMOOTHING (1 << 2)
#define CL_DEBUG_FLAG_DRAW_REAL_LOCAL_POSITION (1 << 3)
#define CL_DEBUG_FLAG_DEBUG_CAMERA (1 << 4)
#define CL_DEBUG_FLAG_DRAW_AABBS (1 << 5)

#define CL_DEBUG_FLAG_VERBOSE_FRAME (1 << 31)

#define CLR_PARTICLE_TYPE_TEST 0
#define CLR_PARTICLE_TYPE_GIB 1

#define CLR_HUD_ITEM_SPAWN_PROMPT (1 << 0)
#define CLR_HUD_ITEM_PLAYER_STATUS (1 << 1)
#define CLR_HUD_ITEM_CROSSHAIR (1 << 2)
#define CLR_HUD_ITEM_DEAD (1 << 3)
#define CLR_HUD_ITEM_TITLE (1 << 4)

struct ClientView
{
    Transform camera;
    //i32 showHud;
    i32 rightHand;
    i32 leftHand;
    i32 textFieldFlags;
    i32 health;
};

struct ClientRenderSettings
{
    u32 debugFlags;
    i32 worldLightsMax;
    i32 extraLightsMax;
    ClientView viewModels;
};

struct ClientRenderer
{
    ZRAssetDB* db;
    ZRParticleEmitter testEmit;
    ZRParticleEmitter gibEmit;
};

// Allocate a renderer
extern "C" ClientRenderer* CLR_Create(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* assetDb, i32 particlesPerPool);
extern "C" void CLR_Shutdown(ClientRenderer* cr);
extern "C" void CLR_SpawnTestParticle(ClientRenderer* cr, i32 type, Vec3 pos, Vec3 vel);
extern "C" void CLR_TickTestParticles(ClientRenderer* cr, timeFloat delta);
/**
 * Write Client state to draw buffers
 */
extern "C" void CLR_WriteDrawFrame(
    ClientRenderer* cr,
    ZRViewFrame* frame,
    SimScene* sim,
    Transform* camera,
    ZRDrawObj* debugObjs,
    i32 numDebugObjs,
    ClientRenderSettings cfg
);

#endif // GAME_DRAW_H