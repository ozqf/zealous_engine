#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <shellapi.h> // for parsing command line tokens
#include <stdio.h>
#include <stdlib.h>

#include "../../internal_headers/zengine_internal.h"

ze_external zErrorCode ZWindow_Init();
