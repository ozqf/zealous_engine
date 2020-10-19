#ifndef ZE_INI_H
#define ZE_INI_H

#include "ze_common.h"
#include "ze_blob_store.h"
#include "ze_byte_buffer.h"
#include "ze_hash.h"
#include "ze_vars.h"

#define ZEINI_DEFAULT_SECTION_NAME "global"

struct ZEIniSection
{
	i32 hash;
	ZEBlobStore store;
};

struct ZEIniField
{
	i32 sectionHash;
	i32 nameHash;
	i32 charHash;
	i32 i;
	f32 f;
};

struct ZEIniFile
{
	ZELookupTable* strTable;
	ZEBuffer strData;
	// linear list of sections atm
	ZEIniSection* sections;
	i32 numSections;
	i32 maxSections;
	i32 defaultSectionHash;
	i32 loadSectionHash;
	
	i32 InternString(const char* sectionName)
	{
		i32 hash = ZE_Hash_djb2((u8*)sectionName);
		i32 curOffset = strTable->FindData(hash);
		if (curOffset != strTable->m_invalidDataValue)
		{
			return hash;
		}
		ZEIntern* header = ZEInternString(strTable, &this->strData, sectionName);
		i32 headerOffset = (u8*)header - this->strData.start;
		strTable->Insert(header->hash, header->headerOffset);
		return header->hash;
	}
	
	i32 HashString(const char* str)
	{
		return ZE_Hash_djb2((u8*)str);
	}

	ZEIniSection* GetSection(i32 hash)
	{
		for (i32 i = 0; i < this->numSections; ++i)
		{
			if (this->sections[i].hash == hash)
			{
				return &this->sections[i];
			}
		}
		return NULL;
	}

	i32 RegisterSection(const char* sectionName)
	{
		i32 strHash = InternString(sectionName);
		ZE_ASSERT(this->numSections < this->maxSections,
			"INI - no section available")
		
		ZEIniSection* section = this->GetSection(strHash);
		if (section != NULL)
		{
			printf("Section %s already registered\n", sectionName);
				return strHash;
		}
		section = &this->sections[this->numSections++];
		section->hash = strHash;
		ZE_InitBlobStore(&section->store, 64, sizeof(ZEIniField), 0);
		return strHash;
	}

	/*
	If field does not exist, it will be added, otherwise it will be patched
	*/
	i32 RegisterField(
		i32 sectionHash,
		const char* fieldName,
		const char* fieldValue,
		i32 bPatchCurrent)
	{
		if (sectionHash == 0) { sectionHash = this->defaultSectionHash; }
		ZEIniSection* section = GetSection(sectionHash);
		if (section == NULL)
		{
			printf("No section %d found for field\n", sectionHash);
			return 0;
		}
		i32 fieldNameHash = this->InternString(fieldName);
		i32 valueHash = this->InternString(fieldValue);
		ZEIniField* field = (ZEIniField*)section->store.GetById(fieldNameHash);
		if (field == NULL)
		{
			field = (ZEIniField*)section->store.GetFreeSlot(fieldNameHash);
		}
		else if (bPatchCurrent == NO)
		{
			// already exists, don't update!
			return field->nameHash;
		}
		field->nameHash = fieldNameHash;
		field->charHash = valueHash;
		field->sectionHash = sectionHash;
		field->f = (f32)atof(fieldValue);
		field->i = ZStr_AsciToInt32(fieldValue);
		return field->nameHash;
	}

	const char* GetString(i32 hash)
	{
		i32 offset = this->strTable->FindData(hash);
		if (offset == this->strTable->m_invalidDataValue)
		{ return NULL; }
		ZEIntern* intern = (ZEIntern*)this->strData.GetAtOffset(offset);
		return (const char*)this->strData.GetAtOffset(intern->charsOffset);
	}

	ZEIniField* GetField(const char* sectionName, const char* fieldName)
	{
		i32 sectionHash = ZE_Hash_djb2((u8*)sectionName);
		ZEIniSection* section = this->GetSection(sectionHash);
		if (section == NULL) { return NULL; }

		i32 fieldHash = ZE_Hash_djb2((u8*)fieldName);
		return (ZEIniField*)section->store.GetById(fieldHash);
	}

	const char* GetFieldStr(const char* sectionName, const char* fieldName)
	{
		ZEIniField* field = GetField(sectionName, fieldName);
		if (field == NULL) { return NULL; }
		return GetString(field->charHash);
	}

	i32 GetFieldi(const char* sectionName, const char* fieldName)
	{
		ZEIniField* field = GetField(sectionName, fieldName);
		if (field == NULL) { return 0; }
		return field->i;
	}

	f32 GetFieldf(const char* sectionName, const char* fieldName)
	{
		ZEIniField* field = GetField(sectionName, fieldName);
		if (field == NULL) { return 0; }
		return field->f;
	}

	void PrintStrings()
	{
		
	}
};

static ZEIniFile* ZEIni_Create()
{
	ZEIniFile* iniFile = (ZEIniFile*)malloc(sizeof(ZEIniFile));
	*iniFile = {};
	i32 strSpace = MegaBytes(1);
	iniFile->strData = Buf_FromMalloc(malloc(strSpace), strSpace);
	iniFile->strTable = ZE_LT_Create(128, -1, NULL);
	iniFile->maxSections = 128;
	iniFile->sections = (ZEIniSection*)malloc(
		sizeof(ZEIniSection) * iniFile->maxSections);
	iniFile->defaultSectionHash = iniFile->RegisterSection(ZEINI_DEFAULT_SECTION_NAME);
	iniFile->loadSectionHash = iniFile->defaultSectionHash;
	return iniFile;
}

static void ZEIni_Destroy(ZEIniFile* file)
{
	ZE_LT_Delete(file->strTable);
	free(file->sections);
	free(file);
}

static void ZEIni_ReadLine(ZEIniFile* file, char* buf)
{
	i32 readIndex = -1;
	// patch out unwanted trailing chars
	readIndex = ZStr_FindFirstCharMatch(buf, '\n');
	if (readIndex != -1)
	{
		buf[readIndex] = '\0';
	}
	i32 len = ZStr_Len(buf);
	// assuming no white space before line
	char c = buf[0];
	if (ZStr_IsCharLetter(c))
	{
		#if 1
		// Line is a variable
		// find '=' key=value splitter
		// patch to line end
		// key=value
		readIndex = ZStr_FindFirstCharMatch(buf, '=');
		if (readIndex >= 0)
		{
			buf[readIndex] = '\0';
			char* valueBuf = &buf[readIndex + 1];
			i32 varLabelLen = ZStr_Len(valueBuf);
			if (varLabelLen > 0)
			{
				file->RegisterField(file->loadSectionHash, buf, valueBuf, YES);
			}
		}
		#endif
	}
	if (c == '[')
	{
		readIndex = ZStr_FindFirstCharMatch(buf, ']');
		if (readIndex > 1)
		{
			buf[readIndex] = '\0';
			char* setName = &buf[1];
			file->loadSectionHash = file->RegisterSection(setName);
		}
	}
}

static void ZEIni_Write(ZEIniFile* file, const char* path)
{
	FILE* f = NULL;
	fopen_s(&f, path, "w");
	if (f == NULL)
	{
		printf("Failed to open %s for writing\n", path);
		return;
	}
	for (i32 i = 0; i < file->numSections; ++i)
	{
		ZEIniSection* sec = &file->sections[i];
		fprintf(f, "[%s]\n", file->GetString(sec->hash));
		ZEBlobArray* arr = sec->store.m_array;
		for (i32 j = 0; j < arr->m_numBlobs; ++j)
		{
			ZEIniField* field = (ZEIniField*)arr->GetByIndex(j);
			fprintf(f, "%s=%s\n",
				file->GetString(field->nameHash),
				file->GetString(field->charHash)
			);
		}
		fprintf(f, "\n");
	}
	i32 written = ftell(f);
	printf("Wrote %d chars to %s\n", written, path);
	fclose(f);
}

#endif // ZE_INI_H