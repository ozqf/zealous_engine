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

struct ClientRenderSettings
{
    u32 debugFlags;
    i32 worldLightsMax;
    i32 extraLightsMax;
};

extern "C" void CLR_Init(ZRAssetDB* db);
extern "C" void CLR_Shutdown();
/**
 * Write Client state to draw buffers
 */
extern "C" void CLR_WriteDrawFrame(
    ZRViewFrame* frame,
    SimScene* sim,
    Transform* camera,
    ZRDrawObj* debugObjs,
    i32 numDebugObjs,
    ClientRenderSettings cfg
);

#endif // CLIENT_RENDER_H