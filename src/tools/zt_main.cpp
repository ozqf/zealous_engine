#include <stdio.h>
#include "tests/ze_tests.h"
#include "zt_monitor.h"
#include "zt_packer.h"

#include "../ze_common/ze_common_full.h"

void main(int argc, char** argv)
{
	printf("Zealous Tools\nArgs (%d): ", argc);
	for (i32 i = 0; i < argc; ++i)
	{
		printf("%d: %s, ", i, argv[i]);
	}
	printf("\n");

	if (argc < 2 || ZStr_Compare("test", argv[1]) == 0)
	{
		ZETests_Run();
	}
	else if (ZStr_Compare("monitor", argv[1]) == 0)
	{
		ZT_MonitorStart();
	}
	else if (ZStr_Compare("packer", argv[1]) == 0)
	{
		ZPack_Run(argc, argv);
	}
	else
	{
		printf("Unknown mode %s\n", argv[1]);
		printf("Try zetools test/monitor/packer\n");
	}
	printf("ZT - done\n");
}