#include "zr_db_internal.h"

static ZRAsset* ZRDB_GetBlobByName(ZRAssetDB* handle, char* name)
{
	ZRDB_CAST_TO_INTERNAL(handle, db)
	i32 index = 0;
	for (i32 i = 0; i < db->num)
}
