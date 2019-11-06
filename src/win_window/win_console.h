#ifndef WIN_CONSOLE_H
#define WIN_CONSOLE_H

#include "../ze_common/ze_common.h"

#define ZE_CONSOLE_BUFFER_SIZE 256

static char g_consoleResultBuffer[ZE_CONSOLE_BUFFER_SIZE];
static char g_consoleInputBuffer[ZE_CONSOLE_BUFFER_SIZE];
static i32 g_consolePosition = 0;

static void Console_Reset()
{
    ZE_SET_ZERO(g_consoleInputBuffer, ZE_CONSOLE_BUFFER_SIZE);
    g_consolePosition = 0;
}

static void Console_Print()
{
    printf("CONSOLE (%d): %s\n", g_consolePosition, g_consoleInputBuffer);
}

/*
Function uses glfw char codes at the moment.
Raw scan codes:
259 - backspace
257 - enter
*/
static char* Console_AddChar(char charCode)
{
    //printf("CONSOLE read code %d\n", charCode);
    if (charCode == 3)
    {
        if (g_consolePosition > 0)
        {
            g_consolePosition--;
        }
        g_consoleInputBuffer[g_consolePosition] = '\0';
        Console_Print();
        return NULL;
    }
    if (charCode == 1)
    {
        ZE_COPY(g_consoleInputBuffer, g_consoleResultBuffer, ZE_CONSOLE_BUFFER_SIZE);
        Console_Reset();
        Console_Print();
        return g_consoleResultBuffer;
    }
    g_consoleInputBuffer[g_consolePosition] = charCode;
    if (g_consolePosition < (ZE_CONSOLE_BUFFER_SIZE - 1))
    {
        g_consolePosition++;
    }
    Console_Print();
    return NULL;
}

#endif // WIN_CONSOLE_H