#ifndef ZE_TEST_STRINGS_H
#define ZE_TEST_STRINGS_H

#include "../../ze_common/ze_common_full.h"
#include "../../ze_common/ze_ini.h"

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
	// "classname" "info_player_start"
	// minimal valid length would be "a" "b" or 8 chars
	// TODO: No handling of " quotes within a string
	if (lineLen < 8)
	{
		printf("Line %s is too short\n", line);
		return;
	}
	i32 quotes = ZE_CountSpecificChar(line, '"');
	if (quotes != 4)
	{
		printf("Counted %d quotes in setting line. Expected %d\n",
			quotes, 4);
		return;
	}
	//printf("Setting: %s\n", line);
	char* keyStart = ZE_ReadToChar((char*)line, '"');
	char* keyEnd = ZE_ReadToChar((char*)keyStart, '"');
	char* valueStart = ZE_ReadToChar((char*)keyEnd, '"');
	char* valueEnd = ZE_ReadToChar((char*)valueStart, '"');
	*(keyEnd - 1) = '\0';
	*(valueEnd - 1) = '\0';
	printf("Field Key: %s value: %s\n", keyStart, valueStart);
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

static char* Test_FindIniVar(ZELookupTable* table, ZEBuffer* data, const char* name)
{
	i32 hash = ZE_Hash_djb2((u8*)name);
	i32 offset = table->FindData(hash);
	if (offset == table->m_invalidDataValue) { return NULL; }
	ZEIntern* intern = (ZEIntern*)data->GetAtOffset(offset);
	return (char*)data->GetAtOffset(intern->charsOffset);
}

static void Test_ReadIni_3_0()
{
	/*
	Ideas for loading:
	all ini files and their sections are loaded into the same pool.
	> Data: Linear buffer of interned strings.
	> Table: hash table where key is section name + field name, and value is
	the offset to the field's value string in the data buffer.
	
	if done this way, no way to go from the data to the section/field names
	to save the file back out again!
	need to save the offsets to the section and field names for each.
	*/
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
	ZEBuffer stringsBuf = Buf_FromMalloc(malloc(fileLen * 4), fileLen * 4);
	ZEBuffer* strings = &stringsBuf;

	ZELookupTable* table = ZE_LT_Create(128, -1, NULL);

	i32 line = 1;
	i32 readIndex = -1;
	while(fgets(buf, bufSize, f))
	{
		// patch out '\n'
		readIndex = ZE_FindFirstCharMatch(buf, '\n');
		if (readIndex != -1)
		{
			buf[readIndex] = '\0';
		}
		i32 len = ZE_StrLen(buf);
		printf("%d: (%d chars) %s\n", line, len, buf);
		//Test_PrintCharCodes(buf);
		char c = buf[0];
		if (Test_IsCharLetter(c))
		{
			#if 0
			// Line is a variable
			// find '=' key=value splitter
			// patch to line end
			// key=value
			readIndex = ZE_FindFirstCharMatch(buf, '=');
			if (readIndex >= 0)
			{
				buf[readIndex] = '\0';
				char* valueBuf = &buf[readIndex + 1];
				i32 varLabelLen = ZE_StrLen(valueBuf);
				printf("Var label len %d\n", varLabelLen);
				if (varLabelLen > 0)
				{
					printf("Key %s, Value %s\n", buf, valueBuf);
					//////////////////////////
					// intern key
					ZE_INIT_PTR_IN_PLACE(key, ZEIntern, strings)
					key->hash = ZE_Hash_djb2((u8*)buf);
					// recalc length since we adjusted it
					key->len = ZE_StrLen(buf);
					key->charsOffset = stringsBuf.CursorOffset();
					strcpy_s((char*)strings->cursor, len, buf);
					strings->cursor += key->len;
					//////////////////////////
					// intern value
					// Record current cursor offset for lookup table
					i32 valueStructOffset = stringsBuf.CursorOffset();
					table->Insert(key->hash, valueStructOffset);
					// create value entry
					ZE_INIT_PTR_IN_PLACE(val, ZEIntern, strings)
					val->hash = ZE_Hash_djb2((u8*)valueBuf);
					val->len = ZE_StrLen(valueBuf);
					val->charsOffset = stringsBuf.CursorOffset();
					strcpy_s((char*)strings->cursor, len, valueBuf);
					strings->cursor += val->len;
				}
			}
			#endif
		}
		if (c == '[')
		{
			readIndex = ZE_FindFirstCharMatch(buf, ']');
			if (readIndex > 1)
			{
				buf[readIndex] = '\0';
				char* setName = &buf[1];
				printf("Set %s\n", setName);
			}
		}
		line++;
	}
	#if 0
	/////////////////////////////
	// Iterate buffer and check
	printf("Bytes written: %d\n", stringsBuf.Written());
	u8* read = stringsBuf.start;
	u8* end = stringsBuf.cursor;
	while (read < end)
	{
		i32 offset = read - stringsBuf.start;
		ZEIntern* intern = (ZEIntern*)read;
		printf("Intern at %d hash %d len %d str: %s\n",
			offset, intern->hash, intern->len, (char*)stringsBuf.GetAtOffset(intern->charsOffset));
		read += sizeof(ZEIntern) + intern->len;
	}
	/////////////////////////////
	// Iterate lookup table and check
	printf("-- Field lookup table --\n");
	for (i32 i = 0; i < table->m_maxKeys; ++i)
	{
		ZELookupKey* key = &table->m_keys[i];
		if (key->id == 0) { continue; }
		ZEIntern* intern = (ZEIntern*)stringsBuf.GetAtOffset(key->data);
		printf("Value for id hash %d: %s\n",
			key->id, (char*)stringsBuf.GetAtOffset(intern->charsOffset));
	}

	char* testKey = "ddddd";
	char* foo = Test_FindIniVar(table, &stringsBuf, testKey);
	if (foo != NULL)
	{
		printf("Lookup test - %s: %s\n", testKey, foo);
	}
	else
	{
		printf("ERROR - look test found no result for field %s\n", testKey);
	}
	#endif

	free(stringsBuf.start);
	printf("\tini test done\n\n");
	fclose(f);
}

static void Test_PrintIniField(ZEIniFile* file, const char* section, const char* field)
{
	printf("Ini section %s field %s: i %d, f %.5f, str: %s\n",
		section,
		field,
		file->GetFieldi(section, field),
		file->GetFieldf(section, field),
		file->GetFieldStr(section, field));
}

static void Test_ReadIni()
{
	/*
	Ideas for loading:
	all ini files and their sections are loaded into the same pool.
	> Data: Linear buffer of interned strings.
	> Table: hash table where key is section name + field name, and value is
	the offset to the field's value string in the data buffer.
	
	if done this way, no way to go from the data to the section/field names
	to save the file back out again!
	need to save the offsets to the section and field names for each.
	*/
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
	// ZEBuffer stringsBuf = Buf_FromMalloc(malloc(fileLen * 4), fileLen * 4);
	// ZEBuffer* strings = &stringsBuf;

	// ZELookupTable* table = ZE_LT_Create(128, -1, NULL);

	ZEIniFile* file = ZEIni_Create();
	i32 sectionHash = file->RegisterSection("global");
	sectionHash = file->RegisterSection("global");
	i32 line = 1;
	i32 readIndex = -1;
	while(fgets(buf, bufSize, f))
	{
		sectionHash = ZEIni_ReadLine(file, sectionHash, buf);
		#if 0
		// patch out '\n'
		readIndex = ZE_FindFirstCharMatch(buf, '\n');
		if (readIndex != -1)
		{
			buf[readIndex] = '\0';
		}
		i32 len = ZE_StrLen(buf);
		//printf("%d: (%d chars) %s\n", line, len, buf);
		//Test_PrintCharCodes(buf);
		char c = buf[0];
		if (Test_IsCharLetter(c))
		{
			#if 1
			// Line is a variable
			// find '=' key=value splitter
			// patch to line end
			// key=value
			readIndex = ZE_FindFirstCharMatch(buf, '=');
			if (readIndex >= 0)
			{
				buf[readIndex] = '\0';
				char* valueBuf = &buf[readIndex + 1];
				i32 varLabelLen = ZE_StrLen(valueBuf);
				//printf("Var label len %d\n", varLabelLen);
				if (varLabelLen > 0)
				{
					file->RegisterField(sectionHash, buf, valueBuf);
					#if 0
					printf("Key %s, Value %s\n", buf, valueBuf);
					//////////////////////////
					// intern key
					ZE_INIT_PTR_IN_PLACE(key, ZEIntern, strings)
					key->hash = ZE_Hash_djb2((u8*)buf);
					// recalc length since we adjusted it
					key->len = ZE_StrLen(buf);
					key->charsOffset = stringsBuf.CursorOffset();
					strcpy_s((char*)strings->cursor, len, buf);
					strings->cursor += key->len;
					//////////////////////////
					// intern value
					// Record current cursor offset for lookup table
					i32 valueStructOffset = stringsBuf.CursorOffset();
					table->Insert(key->hash, valueStructOffset);
					// create value entry
					ZE_INIT_PTR_IN_PLACE(val, ZEIntern, strings)
					val->hash = ZE_Hash_djb2((u8*)valueBuf);
					val->len = ZE_StrLen(valueBuf);
					val->charsOffset = stringsBuf.CursorOffset();
					#endif
				}
			}
			#endif
		}
		if (c == '[')
		{
			readIndex = ZE_FindFirstCharMatch(buf, ']');
			if (readIndex > 1)
			{
				buf[readIndex] = '\0';
				char* setName = &buf[1];
				sectionHash = file->RegisterSection(setName);
				//printf("Set %s\n", setName);
			}
		}
		#endif
		line++;
	}
	printf("--- SECTIONS (%d)---\n", file->numSections);
	for (i32 i = 0; i < file->numSections; ++i)
	{
		ZEIniSection* section = &file->sections[i];
		printf("%d: %s\n", i, file->GetString(section->hash));
	}
	
	Test_PrintIniField(file, "section_a", "player_name");
	Test_PrintIniField(file, "section_b", "player_name");
	Test_PrintIniField(file, "controls", "sensitivity");
	Test_PrintIniField(file, "keys", "move_forward");
	
	/////////////////////////////
	// Iterate buffer and check
	printf("Bytes written: %d of %d\n",
		file->strData.Written(), file->strData.capacity);
	u8* read = file->strData.start;
	u8* end = file->strData.cursor;
	while (read < end)
	{
		i32 offset = read - file->strData.start;
		ZEIntern* intern = (ZEIntern*)read;
		printf("Intern at %d hash %d len %d str: %s\n",
			offset, intern->hash, intern->len, (char*)file->strData.GetAtOffset(intern->charsOffset));
		read += sizeof(ZEIntern) + intern->len;
	}
	const char* outputPath = "config_copy.ini";
	printf("Write test to path %s\n", outputPath);
	ZEIni_Write(file, outputPath);
	#if 0
	/////////////////////////////
	// Iterate lookup table and check
	printf("-- Field lookup table --\n");
	for (i32 i = 0; i < table->m_maxKeys; ++i)
	{
		ZELookupKey* key = &table->m_keys[i];
		if (key->id == 0) { continue; }
		ZEIntern* intern = (ZEIntern*)stringsBuf.GetAtOffset(key->data);
		printf("Value for id hash %d: %s\n",
			key->id, (char*)stringsBuf.GetAtOffset(intern->charsOffset));
	}

	char* testKey = "ddddd";
	char* foo = Test_FindIniVar(table, &stringsBuf, testKey);
	if (foo != NULL)
	{
		printf("Lookup test - %s: %s\n", testKey, foo);
	}
	else
	{
		printf("ERROR - look test found no result for field %s\n", testKey);
	}
	#endif

	//free(stringsBuf.start);
	ZEIni_Destroy(file);
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
	Test_ReadMapFormat();
	Test_ReadIni();
	printf("\tDone\n");
}


#endif // ZE_TEST_STRINGS_H