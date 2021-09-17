
#include "zt_map_converter_internal.h"

static void DebugPrintFace(ZTMapFace* face)
{
	printf("%.1f,\t%.1f,\t%.1f -\t%.1f,\t%.1f,\t%.1f -\t%.1f,\t%.1f,\t%.1f\n",
		face->a.x, face->a.y, face->a.z,
		face->b.x, face->b.y, face->b.z,
		face->c.x, face->c.y, face->c.z);
}

static void DebugPrintFileData(ZTMapBrush* brushes, i32 numBrushes, ZTMapFace* faces, i32 numFaces)
{
	printf("=== Print %d brushes ===\n", numBrushes);
	for (i32 i = 0; i < numBrushes; ++i)
	{
		ZTMapBrush* brush = &brushes[i];
		printf("Brush %d - %d faces\n", i, brush->numFaces);
		
		i32 firstIndex = brush->firstFaceIndex;
		i32 endIndex = firstIndex + brush->numFaces;
		for (i32 j = firstIndex; j < endIndex; ++j)
		{
			ZTMapFace* face = &faces[j];
			printf("%d -\t%.1f,\t%.1f,\t%.1f -\t%.1f,\t%.1f,\t%.1f -\t%.1f,\t%.1f,\t%.1f\n",
				j,
				face->a.x, face->a.y, face->a.z,
				face->b.x, face->b.y, face->b.z,
				face->c.x, face->c.y, face->c.z);
		}
	}
}

static ErrorCode ParseFace(const char* line, i32 lineLen, ZTMapFace* r)
{
	if (r == NULL)
	{
		return 1;
	}
	//printf("Face: %s\n", line);
	// expect 21 tokens, eg:
	// ( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) metal1_3 0 0 0 1 1
	const i32 bufLen = 512;
	const i32 maxTokens = 32;
	if (lineLen > bufLen)
	{
		printf("Line length of %d exceeds limit %d\n",
			lineLen, bufLen); 
		return 1;
	}
	char buf[bufLen];
	char* tokens[maxTokens];
	const i32 expectedTokens = 21;
	i32 numTokens = ZStr_Tokenise(line, buf, tokens, maxTokens);
	if (numTokens != expectedTokens)
	{
		printf("Bad face - expected %d tokens, got %d\n", expectedTokens, numTokens);
		return 1;
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
		return 1;
	}
	#if 0
	printf("Face : ");
	for (i32 i = 0; i < numTokens; ++i)
	{
		printf("%s, ", tokens[i]);
	}
	printf("\n");
	#endif
	r->a.x = (f32)atof(tokens[1]);
	r->a.y = (f32)atof(tokens[2]);
	r->a.z = (f32)atof(tokens[3]);
	
	r->b.x = (f32)atof(tokens[6]);
	r->b.y = (f32)atof(tokens[7]);
	r->b.z = (f32)atof(tokens[8]);
	
	r->c.x = (f32)atof(tokens[11]);
	r->c.y = (f32)atof(tokens[12]);
	r->c.z = (f32)atof(tokens[13]);
	
	// printf("%s: ( %.1f, %.1f, %.1f )", tokens[15], r->a.x, r->a.y, r->a.z);
	// printf("( %.1f, %.1f, %.1f ) ", r->b.x, r->b.y, r->b.z);
	// printf("( %.1f, %.1f, %.1f )\n", r->c.x, r->c.y, r->c.z);
	return 0;
}

static void ParseSetting(const char* line, i32 lineLen)
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

static zErrorCode ParseMapFromText(const char* text)
{
	
}

static ErrorCode ParseMapFromFile(const char* path, ZTMapFile* result)
{
	if (result == NULL)
	{
		return ZE_ERROR_NULL_ARGUMENT;
	}
	const i32 bufSize = 512;
	char buf[bufSize];
	// read line by line in text mode
	FILE* f = NULL;
	// const char* path = "map_format_example_128x128x32_cube.map";
	printf("--- Test read map %s ---\n", path);
	fopen_s(&f, path, "r");
	if (f == NULL)
	{
		printf("Failed to open ini test %s\n", path);
		return ZE_ERROR_MISSING_FILE;
	}
	i32 line = 1;
	i32 i = -1;
	
	ZTMapBrush* brush = NULL;
	
	// array of faces to assign
	const i32 maxFaces = 64;
	ZTMapFace* faces = (ZTMapFace*)malloc(sizeof(ZTMapFace) * maxFaces);
	i32 nextFaceIndex = 0;
	
	// array of brushes to assign
	const i32 maxBrushes = 64;
	ZTMapBrush* brushes = (ZTMapBrush*)malloc(sizeof(ZTMapBrush) * maxBrushes);
	i32 nextBrushIndex = 0;
	
	*result = {};
	result->brushes = brushes;
	result->maxBrushes = maxBrushes;
	result->faces = faces;
	result->maxFaces = maxFaces;
	
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
		if (len >= 2 && lineStart[0] == '/' && lineStart[1] == '/' )
		{ line++; continue; }
		
		i32 bWroteFace = NO;
		//printf("%d: %s\n", line, buf);
		switch (lineStart[0])
		{
			case '{':
			break;
			case '}':
			break;
			case '(':
			{
				if (brush == NULL)
				{
					printf("Starting brush %d\n", nextBrushIndex);
					brush = &brushes[nextBrushIndex++];
					*brush = {};
				}
				
				printf("Assign face %d\n", nextFaceIndex);
				ZTMapFace* face = &faces[nextFaceIndex++];
				*face = {};
				
				ErrorCode brushErr = ParseFace(lineStart, len, face);
				if (brushErr != 0)
				{
					printf("Error code %d reading face\n", brushErr);
					return 1;
				}
				brush->numFaces++;
				bWroteFace = YES;
			} break;
			case '"':
			ParseSetting(lineStart, len);
			break;
			default:
			// FAIL PARSE
			printf("Unexpected first char %c on line %d\n",
				lineStart[0], line);
			fclose(f);
			return ZE_ERROR_DESERIALISE_FAILED;
			break;
		}
		
		// if writing brush and didn't add a face this iteration,
		// finish this brush
		if (brush != NULL && bWroteFace == NO)
		{
			printf("Finished brush with %d faces\n", brush->numFaces);
			brush = NULL;
		}
		
		line++;
	}
	fclose(f);
	
	result->numBrushes = nextBrushIndex;
	result->numFaces = nextFaceIndex;
	
	return ZE_ERROR_NONE;
}
