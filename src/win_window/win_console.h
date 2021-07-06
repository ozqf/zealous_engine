#ifndef WIN_CONSOLE_H
#define WIN_CONSOLE_H

#include "../../headers/common/ze_common.h"

#define ZE_CONSOLE_BUFFER_SIZE 256

#define BACKSPACE_CHAR_CODE 3
#define ENTER_CHAR_CODE 1

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

static char Console_ReplaceShiftCodes(char c, i32 bShiftOn)
{
    // codes will come in upper case, so check if it should be
    // set to lower
    // if (!bShiftOn && c >= 65 && c <= 90)
    // {
    //     c += 32;
    // }
    if (!bShiftOn)
    {
        c = ZStr_CharToLower(c);
    }
    if (bShiftOn)
    {
        switch (c)
        {
            case '1': c = '!'; break;
            case '2': c = '"'; break;
            // case '3': c = '£'; break; // Pound symbol :/
            case '4': c = '$'; break;
            case '5': c = '%'; break;
            case '6': c = '^'; break;
            case '7': c = '&'; break;
            case '8': c = '*'; break;
            case '9': c = '('; break;
            case '0': c = ')'; break;
            case '-': c = '_'; break;
            case '=': c = '+'; break;

            case '\\': c = '|'; break;
            case '/': c = '?'; break;
            case ',': c = '<'; break;
            case '.': c = '>'; break;
            case ';': c = ':'; break;
            case '\'': c = '@'; break;
            case '#': c = '~'; break;
            case '[': c = '{'; break;
            case ']': c = '}'; break;
        }
    }
    return c;
}

/*
Function uses glfw char codes at the moment.
Raw scan codes:
259 - backspace
257 - enter
*/
static char* Console_AddChar(char charCode, i32 bShiftOn)
{
    //printf("CONSOLE read code %d\n", charCode);
    if (charCode == BACKSPACE_CHAR_CODE)
    {
        if (g_consolePosition > 0)
        {
            g_consolePosition--;
        }
        g_consoleInputBuffer[g_consolePosition] = '\0';
        Console_Print();
        return NULL;
    }
    if (charCode == ENTER_CHAR_CODE)
    {
        ZE_COPY(g_consoleInputBuffer, g_consoleResultBuffer, ZE_CONSOLE_BUFFER_SIZE);
        Console_Reset();
        Console_Print();
        return g_consoleResultBuffer;
    }
    
    charCode = Console_ReplaceShiftCodes(charCode, bShiftOn);

    g_consoleInputBuffer[g_consolePosition] = charCode;
    if (g_consolePosition < (ZE_CONSOLE_BUFFER_SIZE - 1))
    {
        g_consolePosition++;
    }
    Console_Print();
    return NULL;
}

#endif // WIN_CONSOLE_H