#ifndef CLIENT_RENDER_H
#define CLIENT_RENDER_H

#include "../../ze_common/ze_common.h"
#include "client.h"
#include "../../sim/sim.h"

struct ClientRenderSettings
{
    u32 debugFlags;
    i32 worldLightsMax;
    i32 extraLightsMax;
};

extern "C" void CLR_Init();
extern "C" void CLR_Shutdown();
/**
 * Write Client state to draw buffers
 */
extern "C" void CLR_WriteDrawFrame(
    ZEByteBuffer* list,
    ZEByteBuffer* data,
    SimScene* sim,
    Transform* camera,
    ClientRenderSettings cfg
);

#endif // CLIENT_RENDER_H