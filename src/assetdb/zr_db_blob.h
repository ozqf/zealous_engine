#include "zr_db_internal.h"

static i32 ZRDB_GetBlobIndexName(ZRAssetDB* handle, char* name)
{
	ZRDB_CAST_TO_INTERNAL(handle, db)
	i32 index = 0;
	for (i32 i = 0; i < db->numBlobs; ++i)
	{
		if (ZStr_Compare(name, db->blobs[i].header.fileName) == 0)
		{
			return i;
		}
	}
	return 0;
}

static ZRDBBlob* ZRDB_GetBlobByName(ZRAssetDB* assetDB, char* name)
{
	i32 i = ZRDB_GetBlobIndexName(assetDB, name);
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
	return &db->blobs[i];
}

static ZRDBBlob* ZRDB_CreateBlob(ZRAssetDB* assetDB, char* name, i32 numBytes)
{
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
	i32 index = db->numBlobs++;
	printf("ZRDB - register blob %d: %s (%dKB)\n", index, name, numBytes / 1024);
	ZRDBBlob* blob = &db->blobs[index];
	*blob = {};
	blob->header.id = db->nextId++;
	blob->header.index = index;
	blob->header.fileName = name;
	blob->header.type = ZRDB_ASSET_TYPE_BLOB;
	void* ptr = COM_Malloc(&db->allocs, numBytes, 0, "ZRDBBlob");
	blob->data = Buf_FromBytes((u8*)ptr, numBytes);
	return blob;
}
