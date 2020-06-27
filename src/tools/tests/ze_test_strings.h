#include "../../ze_common/ze_common_full.h"

static void Test_StringFunctions()
{
	printf("\n=== TEST STRING LIB ===\n");
	const i32 bufSize = 512;
	char buf[bufSize];
	/////////////////////////////////////////////
	// Tokenise
	char* testCommand = "foo.exe -option 42 - bar --skegness seventh eighth";
	const i32 maxTokens = 7;
	char* tokens[maxTokens];

	printf("Tokenising \"%s\" (max tokens %d)\n", testCommand, maxTokens);
	i32 numTokens = ZE_ReadTokens(testCommand, buf, tokens, maxTokens);
	printf("Result:\n");
	for (i32 i = 0; i < numTokens; ++i)
	{
		printf("%d: %s\n", i, tokens[i]);
	}

	char* str2D = "foo\nbar\nhello\ngoodbye";
	i32 x = 0, y = 0;
	ZE_StrMeasure2D(str2D, &x, &y);
	printf("Measured %s as %d, %d\n", str2D, x, y);
}
