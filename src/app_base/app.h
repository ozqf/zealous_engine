#ifndef APP_H
#define APP_H

#include "../ze_common/ze_common_full.h"
#include "../zqf_renderer.h"

/////////////////////////////////////////////////////////////////
// Functions exported to Server/Client
/////////////////////////////////////////////////////////////////
extern "C" void App_SendTo(i32 socketIndex, ZNetAddress* addr, u8* data, i32 dataSize);
extern "C" void App_Log(char* msg, i32 categoryMask);
extern "C" void App_Print(char* msg, i32 categoryMask);
// extern "C" void AppImpl_SetMouseCaptured();
// extern "C" void AppImpl_SetMouseFree();
extern "C" i32 App_IsMouseCaptured();
extern "C" void App_Error(char* msg);
extern "C" void App_DebugBreak();
extern "C" timeFloat App_GetSimFrameInterval();
extern "C" frameInt App_CalcTickInterval(timeFloat seconds);
extern "C" timeFloat App_SampleClock();
extern "C" ScreenInfo App_GetScreenInfo();
extern "C" ZRAssetDB* App_GetAssetDB();

// For debug inspection purposes ONLY!
extern "C" void App_Debug_GetServerSim(void** ptr);

extern "C"
void App_SetPerformanceTime(i32 index, i32 tick, timeFloat milliSeconds);

extern "C"
timeFloat App_GetPerformanceTime(i32 index);

#define APP_MAX_PERFORMANCE_FRAMES 60

#define APP_MAX_PACKET_SIZE 1400

/////////////////////////////////////////////////////////////////
// MACROS
/////////////////////////////////////////////////////////////////
#define APP_MAX_ENTITIES 4096

// Debug timing indices
#define APP_STAT_SV_INPUT 0
#define APP_STAT_SV_SIM 1
#define APP_STAT_SV_OUTPUT 2

#define APP_STAT_CL_INPUT 3
#define APP_STAT_CL_SIM 4
#define APP_STAT_CL_OUTPUT 5
#define APP_STAT_CL_RENDER 6

#define APP_STAT_FRAME_TOTAL 7
#define APP_STAT_AABB_SEARCH 8
#define APP_STAT_RENDER_TOTAL 9
#define APP_STAT_CLIENT_SCENE_BUILD 10
#define APP_STAT_COUNT 11

// --------------------------

#define APP_CLIENT_SYNC_RATE_60HZ 60
#define APP_CLIENT_SYNC_RATE_30HZ 30
#define APP_CLIENT_SYNC_RATE_20HZ 20
#define APP_CLIENT_SYNC_RATE_10HZ 10

#define APP_MAX_USERS 16
#define APP_DEFAULT_SOCKET_INDEX 0
#define APP_SERVER_LOOPBACK_PORT 666
#define APP_CLIENT_LOOPBACK_PORT 667

#define APP_DEFAULT_JITTER_TICKS 2

#define APP_LOG_CATEGORY_SYSTEM (1 << 0)
#define APP_LOG_CATEGORY_GAME (1 << 1)
#define APP_LOG_CATEGORY_NET (1 << 2)
#define APP_LOG_CATEGORY_RENDER (1 << 3)
#define APP_LOG_CATEGORY_INPUT (1 << 4)

// comment out to disable logging/printing by app layer
//#define APP_FULL_LOGGING

#ifdef APP_FULL_LOGGING

#define APP_LOG(messageBufSize, format, ...) \
{ \
    char appLogBuf[##messageBufSize##]; \
    sprintf_s(appLogBuf, messageBufSize##, format##, ##__VA_ARGS__##); \
    App_Log(appLogBuf, APP_LOG_CATEGORY_GAME); \
}

#define APP_PRINT(messageBufSize, format, ...) \
{ \
    char appLogBuf[##messageBufSize##]; \
    sprintf_s(appLogBuf, messageBufSize##, format##, ##__VA_ARGS__##); \
    App_Print(appLogBuf, APP_LOG_CATEGORY_GAME); \
}
#else
#define APP_LOG(messageBufSize, format, ...)
#define APP_PRINT(messageBufSize, format, ...)

#endif // APP_FULL_LOGGING

/////////////////////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////////////////////

struct AppPerformanceStat
{
    char* label;
    timeFloat frames[APP_MAX_PERFORMANCE_FRAMES];
    timeFloat Sum()
    {
        timeFloat total = 0.0f;
        for (i32 i = 0; i < APP_MAX_PERFORMANCE_FRAMES; ++i)
        {
            total += frames[i];
        }
        return total / APP_MAX_PERFORMANCE_FRAMES;
    }
};

class AppTimer
{
    public:
    f64 start;
    i32 statIndex;
    i32 statTick;

    AppTimer(i32 index, i32 tick)
    {
        statIndex = index;
        statTick = tick;
        start = App_SampleClock();
    }
    ~AppTimer()
    {
        f64 end = App_SampleClock();
        f32 val = (f32)(end - start) / 1000.0f;
        App_SetPerformanceTime(statIndex, statTick, val);
    }
};

#endif // APP_H