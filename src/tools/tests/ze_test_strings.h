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

static void Test_ParseFace(const char* line, i32 lineLen)
{
	//printf("Face: %s\n", line);
	// expect 21 tokens, eg:
	// ( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) metal1_3 0 0 0 1 1
	const i32 bufLen = 512;
	const i32 maxTokens = 32;
	if (lineLen > bufLen)
	{
		printf("Line length of %d exceeds limit %d\n",
			lineLen, bufLen); 
		return;
	}
	char buf[bufLen];
	char* tokens[maxTokens];
	const i32 expectedTokens = 21;
	i32 numTokens = ZE_ReadTokens(line, buf, tokens, maxTokens);
	if (numTokens != expectedTokens)
	{
		printf("Bad face - expected %d tokens, got %d\n", expectedTokens, numTokens);
		return;
	}
	if (*tokens[0] != '('
		|| *tokens[4] != ')'
		|| *tokens[5] != '('
		|| *tokens[9] != ')'
		|| *tokens[10] != '('
		|| *tokens[14] != ')'
		)
	{
		printf("\nBad token in line - Skipped\n");
		return;
	}
	#if 0
	printf("Face : ");
	for (i32 i = 0; i < numTokens; ++i)
	{
		printf("%s, ", tokens[i]);
	}
	printf("\n");
	#endif
	Vec3 a;
	a.x = (f32)atof(tokens[1]);
	a.y = (f32)atof(tokens[2]);
	a.z = (f32)atof(tokens[3]);
	Vec3 b;
	b.x = (f32)atof(tokens[6]);
	b.y = (f32)atof(tokens[7]);
	b.z = (f32)atof(tokens[8]);
	Vec3 c;
	c.x = (f32)atof(tokens[11]);
	c.y = (f32)atof(tokens[12]);
	c.z = (f32)atof(tokens[13]);
	printf("%s: ( %.3f, %.3f, %3f )", tokens[15], a.x, a.y, a.z);
	printf("( %.3f, %.3f, %3f ) ", b.x, b.y, b.z);
	printf("( %.3f, %.3f, %3f )\n", c.x, c.y, c.z);
}

static void Test_ParseSetting(const char* line, i32 lineLen)
{
	printf("Setting: %s\n", line);
}

static ErrorCode Test_ReadMapFormat()
{
	const i32 bufSize = 512;
	char buf[bufSize];
	// read line by line in text mode
	FILE* f = NULL;
	const char* path = "map_format_example_128x128x32_cube.map";
	printf("--- Test read map %s ---\n", path);
	fopen_s(&f, path, "r");
	if (f == NULL)
	{
		printf("Failed to open ini test %s\n", path);
		return ZE_ERROR_MISSING_FILE;
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
		// eat whitespace
		char* lineStart = ZE_EatWhiteSpace(buf);
		// detect comments
		i32 len = ZE_StrLen(buf);
		if (len >= 2 && lineStart[0] == '/' && lineStart[1] == '/' ) { line++; continue; }
		//printf("%d: %s\n", line, buf);
		switch (lineStart[0])
		{
			case '{':
			break;
			case '}':
			break;
			case '(':
			Test_ParseFace(lineStart, len);
			break;
			case '"':
			Test_ParseSetting(lineStart, len);
			break;
			default:
			// FAIL PARSE
			printf("Unexpected first char %c on line %d\n",
				lineStart[0], line);
			fclose(f);
			return ZE_ERROR_DESERIALISE_FAILED;
			break;
		}
		line++;
	}
	fclose(f);
	return ZE_ERROR_NONE;
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

	fseek(f, 0, SEEK_END);
	i32 fileLen = ftell(f);
	printf("File length: %d\n", fileLen);
	fseek(f, 0, SEEK_SET);
	ZEBuffer varsBuffer = Buf_FromMalloc(malloc(fileLen * 4), fileLen * 4);
	ZEBuffer* vars = &varsBuffer;

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
		printf("%d: (%d chars) %s\n", line, len, buf);
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
				char* valueBuf = &buf[i + 1];
				i32 varLabelLen = ZE_StrLen(valueBuf);
				printf("Var label len %d\n", varLabelLen);
				if (varLabelLen > 0)
				{
					printf("Key %s, Value %s\n", buf, valueBuf);
					ZE_INIT_PTR_IN_PLACE(key, ZEIntern, vars)
					key->hash = ZE_Hash_djb2((u8*)buf);
					// recalc length since we adjusted it
					key->len = ZE_StrLen(buf);
					key->charsOffset = varsBuffer.CursorOffset();
					strcpy_s((char*)vars->cursor, len, buf);
					vars->cursor += key->len;

					ZE_INIT_PTR_IN_PLACE(val, ZEIntern, vars)
					val->hash = ZE_Hash_djb2((u8*)valueBuf);
					val->len = ZE_StrLen(valueBuf);
					val->charsOffset = varsBuffer.CursorOffset();
					strcpy_s((char*)vars->cursor, len, valueBuf);
					vars->cursor += val->len;
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

	// check loaded buffer
	printf("Bytes written: %d\n", varsBuffer.Written());
	u8* read = varsBuffer.start;
	u8* end = varsBuffer.cursor;
	while (read < end)
	{
		i32 offset = read - varsBuffer.start;
		ZEIntern* intern = (ZEIntern*)read;
		printf("Intern at %d hash %d len %d str: %s\n",
			offset, intern->hash, intern->len, (char*)varsBuffer.GetAtOffset(intern->charsOffset));
		read += sizeof(ZEIntern) + intern->len;
	}
	free(varsBuffer.start);
	printf("\tini test done\n\n");
	fclose(f);
}

static void Test_StringFunctions()
{
	printf("\n=== TEST STRING LIB ===\n");
	#if 0
	Test_StringMeasure();
	Test_ReadTokens();
	Test_FileTokenise();
	Test_StringStack();
	#endif
	Test_ReadIni();
	Test_ReadMapFormat();
	printf("\tDone\n");
}


#endif // ZE_TEST_STRINGS_H