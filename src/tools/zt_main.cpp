#include <stdio.h>
#include "tests/ze_tests.h"
#include "zt_monitor.h"

#include "../ze_common/ze_common_full.h"

void main(int argc, char** argv)
{
	printf("Hello, World!\nArgs (%d): ", argc);
	for (i32 i = 0; i < argc; ++i)
	{
		printf("%s, ", argv[i]);
	}
	printf("\n");

	if (argc < 2 || ZE_CompareStrings("test", argv[1]) == 0)
	{
		ZETests_Run();
	}
	else if (ZE_CompareStrings("monitor", argv[1]) == 0)
	{
		ZT_MonitorStart();
	}
	else
	{
		printf("Unknown mode %s\n", argv[1]);
	}
}