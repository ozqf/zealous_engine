#ifndef SERVER_INTERNAL_H
#define SERVER_INTERNAL_H

#include "server.h"

struct SVEntityFrame
{
    i32 latestFrame;
    Vec3 pos;
};

#define SV_EMPTY_DYNAMIC_SCENE -1
#define SV_DEFAULT_DYNAMIC_SCENE 5

#define SV_DEBUG_TIMING (1 << 0)
#define SV_DEBUG_USER_BANDWIDTH (1 << 1)
#define SV_DEBUG_PERFORMANCE (1 << 2)

#define SV_MAX_MALLOCS 1024

#define SV_PACKET_RELIABLE_MAX_BYTES 700

#define SV_CMD_SLOW_RESEND_ATTEMPTS 3
#define SV_CMD_RESEND_WAIT_TICKS 3

internal MallocItem g_mallocItems[SV_MAX_MALLOCS];
internal MallocList g_mallocs;
internal UserList g_users;
internal SimScene g_sim;
internal i32 g_staticSceneIndex = 0;

internal i32 g_isRunning = 0;
internal timeFloat g_elapsed = 0;
internal i32 g_lagCompensateProjectiles = 1;
internal i32 g_unreliableProjectileDeaths = 1;

internal i32 g_maxSyncRate = APP_CLIENT_SYNC_RATE_60HZ;

internal i32 g_debugFlags = 0
    //| SV_DEBUG_TIMING 
    | SV_DEBUG_USER_BANDWIDTH
;
/*
Record entity states for lag compensation rewind
Local entities are not compensated.
Access by Frame number, then entity slot.
*/
#define SV_NUM_POSITION_FRAMES_RECORDED 60
internal SVEntityFrame* g_entityRecords = NULL;

internal void SVConnection_HandleRequest(PacketDescriptor* p);

#endif // SERVER_INTERNAL_H