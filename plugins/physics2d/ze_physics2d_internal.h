#ifndef ZE_PHYSICS2D_INTERNAL_H
#define ZE_PHYSICS2D_INTERNAL_H
/*
2D physics box2d implementation - private header
*/
#include "../ze_physics2d.h"

// Box2d must be built in 64 bit, using embedded standard library and exception disabled.
#include "../../lib/box2d/box2d.h"

/*#ifndef ZP_ASSERT
#define ZP_ASSERT(expression, msg)											\
	if (!(expression))														\
	{																		\
		char assertBuf[512];												\
		ZP_CrashDump();														\
		snprintf(assertBuf, 512, "%s, %d: %s\n", __FILE__, __LINE__, msg);	\
		ZE_Fatal(assertBuf);												\
	}
#endif*/

#endif ZE_PHYSICS2D_INTERNAL_H
