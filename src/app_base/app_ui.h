#ifndef APP_UI_H
#define APP_UI_H

#include "../ui/zui.h"

extern "C" void AppUI_Init();
extern "C" ErrorCode AppUI_WriteFrame(ZEByteBuffer* buf);
extern "C" void AppUI_Update(Vec2 mouseScreenPos);

#endif // APP_UI_H