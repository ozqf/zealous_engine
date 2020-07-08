#ifndef CLIENT_RENDER_H
#define CLIENT_RENDER_H

/**
 * Public interface to client render module
 * > Initialises ZRViewFrame for draw lists
 * > Adds objects in the given Sim to the render list.
 */
#include "../../ze_common/ze_common.h"
#include "client.h"
#include "../../sim/sim.h"
#include "../../assetdb/zr_asset_db.h"

#define CLR_PARTICLE_TYPE_TEST 0
#define CLR_PARTICLE_TYPE_GIB 1

struct ClientRenderSettings
{
    u32 debugFlags;
    i32 worldLightsMax;
    i32 extraLightsMax;
};

struct ClientRenderer
{
    ZRAssetDB* db;
    ZRParticleEmitter testEmit;
    ZRParticleEmitter gibEmit;
};

// Allocate a renderer
extern "C" ClientRenderer* CLR_Create(ZRAssetDB* assetDb, i32 particlesPerPool);
//extern "C" void CLR_Init(ZRAssetDB* db);
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

#endif // CLIENT_RENDER_H