#ifndef ZE_TEST_STRINGS_H
#define ZE_TEST_STRINGS_H

#include "../../../headers/common/ze_common_full.h"
#include "../../../headers/common/ze_ini.h"

/////////////////////////////////////////////
// Tokenise
/////////////////////////////////////////////
static const char* g_testIniTextFile =
"str classname=mob_base\r\n"
"int health=100\r\n";

static void Test_FileTokenise()
{
	printf("\nTest Ini File Tokenise\n");
	// get and measure source text
	const char* inputRaw = g_testIniTextFile;
	i32 len = ZStr_Len(g_testIniTextFile);
	// allocate store for parsed text
	char* inputTemp = (char*)malloc(len);
	// allocate array to store tokens
	const i32 maxTokens = 1024;
	char** tokens = (char**)malloc(sizeof(char*) * maxTokens);
	ZE_SET_ZERO(tokens, sizeof(char*) * maxTokens);

	// tokenise
	i32 numTokens = ZStr_Tokenise(inputRaw, inputTemp, tokens, maxTokens);
	i32 numReplaced = ZStr_ReplaceChars(inputTemp, len, "\r\n", 2, "\0\0", 2);
	printf("Read %d tokens\n", numTokens);
	
	// cleanup
	free(tokens);
	free(inputTemp);
	printf("\tDone\n");
}

static void Test_StringMeasure()
{
	printf("\n--- Test String Measure 2D ---\n");
	/////////////////////////////////////////////
	//  Measure the dimensions of a block of text
	// for 2D alignment
	char* str2D = "foo\nbar\nhello\ngoodbye";
	i32 x = 0, y = 0;
	ZStr_Measure2D(str2D, &x, &y);
	printf("Measured string:\n%s\n\tas %d, %d\n", str2D, x, y);
	char* str = "1\t2\t3\r\n";
	i32 lineLen = ZStr_MeasureLine(str);
	printf("Measured \"%s\" as %d chars\n", str, lineLen);
}

static void Test_CompareExtensions(
	const char* testStr, const char* testExt, i32 bExpected, i32* numTests, i32* passes)
{
	*numTests += 1;
	i32 bResult = ZStr_CheckExtension(testStr, ZStr_Len(testStr), testExt);
	if (bResult != bExpected)
	{
		printf("TEST FAILED - expected %d got %d\n\tStr: %s, test extension %s\n",
			bExpected, bResult, testStr, testExt);
	}
	else
	{
		*passes += 1;
	}
}

static i32 Test_CheckExtension()
{
	printf("\n--- Test Check Extension ---\n");
	i32 numTests = 0;
	i32 passes = 0;
	Test_CompareExtensions("file.txt", ".txt", 1, &numTests, &passes);
	Test_CompareExtensions("file.tXT", ".Txt", 1, &numTests, &passes);
	Test_CompareExtensions("file.obj", ".Txt", 0, &numTests, &passes);
	Test_CompareExtensions("file.long_extension", ".long_extension", 1, &numTests, &passes);
	Test_CompareExtensions("some.file.txt", ".txt", 1, &numTests, &passes);
	Test_CompareExtensions("no_extension", ".txt", 0, &numTests, &passes);
	Test_CompareExtensions(".only_ext", ".only_ext", 1, &numTests, &passes);
	Test_CompareExtensions("blank_extension", "", 0, &numTests, &passes);
	printf("%d of %d passed\n", passes, numTests);
	return 0;
}

static void Test_ReadTokens()
{
	const i32 bufSize = 512;
	char buf[bufSize];
	/////////////////////////////////////////////
	// Tokenise
	char* testCommand = "foo.exe -option 42 - bar --skegness seventh eighth";
	const i32 maxTokens = 7;
	char* tokens[maxTokens];
	printf("--- Tokenise ---\n");
	printf("Tokenising \"%s\" (max tokens %d)\n", testCommand, maxTokens);
	i32 numTokens = ZStr_Tokenise(testCommand, buf, tokens, maxTokens);
	printf("Result:\n");
	for (i32 i = 0; i < numTokens; ++i)
	{
		printf("%d: %s\n", i, tokens[i]);
	}
}

static void Test_StringFunctions()
{
	printf("\n=== TEST STRING LIB ===\n");
	Test_StringMeasure();
	Test_ReadTokens();
	Test_FileTokenise();
	Test_CheckExtension();
	printf("\tString tests done\n");
}


#endif // ZE_TEST_STRINGS_H