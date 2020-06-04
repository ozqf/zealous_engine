#ifndef APP_UI_CPP
#define APP_UI_CPP

#include "app.h"
#include "../ui/zui.h"

extern "C" void AppUI_Init()
{
	APP_PRINT(64, "App UI Init\n");
}

extern "C" void AppUI_Update(Vec2 mouseScreenPos)
{
	
}

extern "C" ErrorCode AppUI_WriteFrame(ZEByteBuffer* buf)
{
	return ZE_ERROR_NONE;
}

#endif // APP_UI_CPP