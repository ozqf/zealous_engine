#include <stdio.h>
#include "tests/ze_tests.h"
#include "zt_monitor.h"

void main(int argc, char** argv)
{
	printf("Hello, World!\n");
	ZETests_Run();
	ZT_MonitorStart();
}