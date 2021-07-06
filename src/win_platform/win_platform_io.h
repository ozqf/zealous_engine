
#include "../../headers/common/ze_common_full.h"

#include <windows.h>
#include <stdio.h>

extern "C" void WinIO_DumpHandles();
extern "C" ZEFileIO WinIO_Init(const char *baseDir, const char *appDir);
