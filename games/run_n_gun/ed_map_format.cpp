#include "ed_map_format.h"

static const char* g_map_1 =
"-13, 7 to 13, 7\n"
"-13, -8 to 13, -8\n"
"-14, -8 to -14, 7\n"
"14, -8 to 14, 7\n"
"\n"
"-13, -5 to -10, -5\n"
"10, -5 to 13, -5\n"
"-3, -5 to 3, -5\n"
;

ze_external ZEBuffer Map2dWriteAscii(ZEngine* ze, Map2d* map)
{
	ZEBuffer buf = Buf_FromMalloc(ze->system.Malloc, MegaBytes(1));
	return buf;
}
