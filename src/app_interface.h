#pragma once
//////////////////////////////////////////////////////////////////////
// App data visible to platform
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "../common/common.h"
#include "sys_events.h"
#include "platform_interface.h"

//////////////////////////////////////////////////////////////////////
// Game Event types
//////////////////////////////////////////////////////////////////////

// Contains pointers to App functions
struct AppInterface
{
    i32     isValid;
    i32     (*AppInit)();
    i32     (*AppShutdown)();
    i32     (*AppRendererReloaded)();
    void    (*AppInput)(PlatformTime* time, ByteBuffer commands);
    void    (*AppUpdate)(PlatformTime* time);
	void    (*AppRender)(PlatformTime* time, ScreenInfo info);
    u8      (*AppParseCommandString)(char* str, char** tokens, i32 numTokens);
};

/*****************************************************
Platform calls DLL with platform function pointers, DLL should return pointers
*****************************************************/
typedef AppInterface (Func_LinkToApp)(AppPlatform);

// eg. in SDL:
// Func_LinkToApp* linkToApp = (Func_LinkToApp *)SDL_LoadFunction(gameModule, "LinkToApp");
