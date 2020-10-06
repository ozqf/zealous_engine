#ifndef ZE_TEST_STRINGS_H
#define ZE_TEST_STRINGS_H

#include "../../ze_common/ze_common_full.h"

/////////////////////////////////////////////
// string stack
/////////////////////////////////////////////

static void Test_ListStringStack(ZEBuffer* buf)
{
	printf("--- String stack ---\n");
	u8* read = buf->start;
	u8* end = buf->cursor;
	while (read < end)
	{
		ZEStringStackItem* item = (ZEStringStackItem*)read;
		if (item->sentinel != ZE_SENTINEL)
		{
			printf("Desync\n");
			return;
		}
		read += sizeof(ZEStringStackItem) + item->len;
		printf("-- Str (%d chars, hash %d) --\n%s\n",
			item->len, item->hash, item->chars);
	}
}

static void Test_StringStack()
{
	printf("\n--- Test String stack ---\n");
	i32 capacity = MegaBytes(1);
	ZEBuffer buf = Buf_FromMalloc(malloc(capacity), capacity);

	ZEStringStackItem* item = NULL;
	item = ZE_AddStackString(&buf, "This is\nsome\nrandom\nstring\n");
	item = ZE_AddStackString(&buf, "This is another random string\n");
	Test_ListStringStack(&buf);
}

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
	i32 len = ZE_StrLen(g_testIniTextFile);
	// allocate store for parsed text
	char* inputTemp = (char*)malloc(len);
	// allocate array to store tokens
	const i32 maxTokens = 1024;
	char** tokens = (char**)malloc(sizeof(char*) * maxTokens);
	ZE_SET_ZERO(tokens, sizeof(char*) * maxTokens);

	// tokenise
	i32 numTokens = ZE_ReadTokens(inputRaw, inputTemp, tokens, maxTokens);
	i32 numReplaced = ZE_ReplaceChars(inputTemp, len, "\r\n", 2, "\0\0", 2);
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
	ZE_StrMeasure2D(str2D, &x, &y);
	printf("Measured string:\n%s\n\tas %d, %d\n", str2D, x, y);
	char* str = "1\t2\t3\r\n";
	i32 lineLen = ZE_StrMeasureLine(str);
	printf("Measured \"%s\" as %d chars\n", str, lineLen);
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
	i32 numTokens = ZE_ReadTokens(testCommand, buf, tokens, maxTokens);
	printf("Result:\n");
	for (i32 i = 0; i < numTokens; ++i)
	{
		printf("%d: %s\n", i, tokens[i]);
	}
}

static void Test_PrintCharCodes(char* str)
{
	while (*str)
	{
		printf("%d, ", *str);
		str++;
	}
	printf("\n");
}

static i32 Test_IsCharLetter(char c)
{
	if (c >= 65 && c <= 90) { return YES; }
	if (c >= 97 && c <= 122) { return YES; }
	return NO;
}

static void Test_ReadMapFormat()
{
	const i32 bufSize = 512;
	char buf[bufSize];
	// read line by line in text mode
	FILE* f = NULL;
	const char* path = "map_format_example_128x128x32_cube.map";
	printf("--- Test read map %s ---\n", path);
	printf("//r char code is %d\n", '\r');
	printf("//n char code is %d\n", '\n');
	fopen_s(&f, path, "r");
	if (f == NULL)
	{
		printf("Failed to open ini test %s\n", path);
		return;
	}
	i32 line = 1;
	i32 i = -1;
	while(fgets(buf, bufSize, f))
	{
		// patch out '\n'
		i = ZE_FindFirstCharMatch(buf, '\n');
		if (i != -1)
		{
			buf[i] = '\0';
		}
		printf("%d: %s", i, buf);
	}
}

static void Test_ReadIni()
{
	const i32 bufSize = 512;
	char buf[bufSize];
	// read line by line in text mode
	FILE* f = NULL;
	//const char* path = "map_format_example_128x128x32_cube.map";
	const char* path = "config.ini";
	printf("--- Test read ini %s ---\n", path);
	printf("//r char code is %d\n", '\r');
	printf("//n char code is %d\n", '\n');
	fopen_s(&f, path, "r");
	if (f == NULL)
	{
		printf("Failed to open ini test %s\n", path);
		return;
	}
	i32 line = 1;
	i32 i = -1;
	while(fgets(buf, bufSize, f))
	{
		// patch out '\n'
		i = ZE_FindFirstCharMatch(buf, '\n');
		if (i != -1)
		{
			buf[i] = '\0';
		}
		i32 len = ZE_StrLen(buf);
		//if (buf[len - 2] == '\r') { printf("line-feed!\n"); }
		//printf("%d: (%d chars) %s\n", line, len, buf);
		//Test_PrintCharCodes(buf);
		char c = buf[0];
		if (Test_IsCharLetter(c))
		{
			// Line is a variable
			// find '=' key=value splitter
			// patch to line end
			// key=value
			i = ZE_FindFirstCharMatch(buf, '=');
			if (i >= 0)
			{
				buf[i] = '\0';
				char* varBuf = &buf[i + 1];
				i32 varLabelLen = ZE_StrLen(varBuf);
				if (varLabelLen > 0)
				{
					printf("Key %s, Value %s\n", buf, varBuf);
				}
			}
		}
		if (c == '[')
		{
			i = ZE_FindFirstCharMatch(buf, ']');
			if (i > 1)
			{
				buf[i] = '\0';
				char* setName = &buf[1];
				printf("Set %s\n", setName);
			}
		}
		line++;
	}
	fclose(f);
}

static void Test_StringFunctions()
{
	printf("\n=== TEST STRING LIB ===\n");
	
	Test_StringMeasure();
	Test_ReadTokens();
	Test_FileTokenise();
	Test_StringStack();
	Test_ReadIni();
	Test_ReadMapFormat();
	printf("\tDone\n");
}


#endif // ZE_TEST_STRINGS_H
