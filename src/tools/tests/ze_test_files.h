
#include "../../ze_common/ze_common_full.h"
#include "../../ze_common/ze_ini.h"


static void Test_PrintCharCodes(char* str)
{
	while (*str)
	{
		printf("%d, ", *str);
		str++;
	}
	printf("\n");
}

////////////////////////////////////////////////////////
// Read .map file
////////////////////////////////////////////////////////

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
	i32 numTokens = ZStr_Tokenise(line, buf, tokens, maxTokens);
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
	i32 quotes = ZStr_CountSpecificChar(line, '"');
	if (quotes != 4)
	{
		printf("Counted %d quotes in setting line. Expected %d\n",
			quotes, 4);
		return;
	}
	//printf("Setting: %s\n", line);
	char* keyStart = ZStr_ReadToChar((char*)line, '"');
	char* keyEnd = ZStr_ReadToChar((char*)keyStart, '"');
	char* valueStart = ZStr_ReadToChar((char*)keyEnd, '"');
	char* valueEnd = ZStr_ReadToChar((char*)valueStart, '"');
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
		i = ZStr_FindFirstCharMatch(buf, '\n');
		if (i != -1)
		{
			buf[i] = '\0';
		}
		// eat whitespace
		char* lineStart = ZStr_EatWhiteSpace(buf);
		// detect comments
		i32 len = ZStr_Len(buf);
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

////////////////////////////////////////////////////////
// Read .ini file
////////////////////////////////////////////////////////

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
	
	const char* path = "config.ini";
	printf("--- Test read ini %s ---\n", path);
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

	ZEIniFile* file = ZEIni_Create();
	i32 line = 1;
	i32 readIndex = -1;
	while(fgets(buf, bufSize, f))
	{
		ZEIni_ReadLine(file, buf);
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
	// printf("Write test to path %s\n", outputPath);
	ZEIni_Write(file, outputPath);
	
	//free(stringsBuf.start);
	ZEIni_Destroy(file);
	printf("\tini test done\n\n");
	fclose(f);
}

////////////////////////////////////////////////////////
// Read .obj model file
////////////////////////////////////////////////////////

static void Test_ReadObjFace(const char* a, const char* b, const char* c)
{
    //printf("Read face %s, %s, %s\n", a, b, c);
}

static void Test_ReadObjectModelFile()
{
    const i32 bufSize = 512;
	char lineBuf[bufSize];
    char tokenBuf[bufSize];
	// read line by line in text mode
	FILE* f = NULL;
	
	const char* path = "i_wing.obj";
	printf("--- Test read .obj model %s ---\n", path);
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
    i32 numVerts = 0;
    i32 numFaces = 0;
    i32 numUVs = 0;
    i32 numNormals = 0;
    i32 lineCount = 0;

    while(fgets(lineBuf, bufSize, f))
	{
        lineCount++;
        // trim line
        char* line = ZStr_EatWhiteSpace(lineBuf);
		ZStr_ReplaceFirst(lineBuf, '\n', '\0');
        i32 numTokens = 0;
        char* tokens[32];
        numTokens = ZStr_Tokenise(line, tokenBuf, tokens, 32);
        if (numTokens < 2) { continue; }
        if (tokens[0][0] == '#') { continue; }

        if (tokens[0][0] == 'o')
        {
            printf("Object %s\n", tokens[1]);
        }
        else if (ZStr_Compare(tokens[0], "v") == 0)
        {
            // eg
            // v 2.070267 -0.157868 0.392587
            numVerts++;
            if (numTokens != 4)
            { printf("Bad token count on line %d\n", lineCount); continue; }
            Vec3 v;
            v.x = (f32)atof(tokens[1]);
            v.y = (f32)atof(tokens[2]);
            v.z = (f32)atof(tokens[3]);
        }
        else if (ZStr_Compare(tokens[0], "f") == 0)
        {
            // eg
            // f 7/15/11 14/25/11 8/16/11
            numFaces++;
            if (numTokens != 4)
            { printf("Bad token count on line %d\n", lineCount); continue; }
            Test_ReadObjFace(tokens[1], tokens[2], tokens[3]);
        }
        else if (ZStr_Compare(tokens[0], "vt") == 0)
        {
            // eg
            // vt 0.625000 0.750000
            numUVs++;
            if (numTokens != 3)
            { printf("Bad token count on line %d\n", lineCount); continue; }
            Vec2 uv;
            uv.x = (f32)atof(tokens[1]);
            uv.y = (f32)atof(tokens[2]);
        }
        else if (ZStr_Compare(tokens[0], "vn") == 0)
        {
            // eg
            // vn -0.7985 0.6019 0.0000
            numNormals++;
            if (numTokens != 4)
            { printf("Bad token count on line %d\n", lineCount); continue; }
            Vec3 n;
            n.x = (f32)atof(tokens[1]);
            n.y = (f32)atof(tokens[2]);
            n.z = (f32)atof(tokens[3]);
        }
        else
        {
            printf("Bad line (%d tokens): %s\n", numTokens, line);
            continue;
        }
        
	}
    printf("Verts %d, faces %d, UVs %d, normals %d\n",
        numVerts, numFaces, numUVs, numNormals);

    fclose(f);
}

static void Test_FileLoading()
{
    printf("\n=== TEST FILE LOADING ===\n");
    Test_ReadMapFormat();
	#if 0
	
	Test_ReadIni();
	Test_ReadObjectModelFile();
	#endif
}
