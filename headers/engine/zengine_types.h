#ifndef ZENGINE_TYPES_H
#define ZENGINE_TYPES_H
#include "../ze_common.h"

struct ZEFrameTimeInfo
{
	i32 frameRate;
	frameInt frameNumber;
	f64 interval;
	// f64 timeSinceStart;
};

struct ZScreenInfo
{
	i32 width;
	i32 height;
	f32 aspect;
};

///////////////////////////////////////////////////////////
// Colours
///////////////////////////////////////////////////////////

union ColourU32
{
	u8 array[4];
	struct
	{
		u8 red, green, blue, alpha;
	};
	struct
	{
		u8 r, g, b, a;
	};
	struct
	{
		u32 value;
	};
};


union ColourF32
{
	f32 array[4];
	struct
	{
		f32 red, green, blue, alpha;
	};
	struct
	{
		f32 r, g, b, a;
	};
};

///////////////////////////////////////////////////////////
// Asset data types
///////////////////////////////////////////////////////////

#define ZE_ASSET_TYPE_NONE 0
#define ZE_ASSET_TYPE_TEXTURE 1
#define ZE_ASSET_TYPE_MESH 2
#define ZE_ASSET_TYPE_MATERIAL 3
#define ZE_ASSET_TYPE_BLOB 4

struct ZRAsset
{
	i32 id;
	i32 index;
	i32 type;
	// Data has changed - needs to be re-uploaded
	i32 bIsDirty;
	zeSize totalSize;
	char *fileName;
	i32 sentinel;
};

struct ZRBlobAsset
{
	ZRAsset header;
	void* ptrA;
	zeSize sizeA;
	void* ptrB;
	zeSize sizeB;
};

struct ZRMaterial
{
	ZRAsset header;
	i32 programId;
	i32 diffuseTexId;
	i32 emissionTexId;
};

struct ZRTexture
{
	ZRAsset header;
	ColourU32 *data;
	i32 width;
	i32 height;
};

struct ZRQuad
{
	i32 textureId;
	Vec2 offset;
	Vec2 uvMin;
	Vec2 uvMax;
	f32 radians;
};

struct ZRLineVertex
{
	Vec3 pos;
	Vec3 colour;
	f32 thickness;
};

#define VEC3_SIZE = 12
#define VEC2_SIZE = 8

// Currently stores every vertex, no sharing
struct ZRMeshData
{
	u32 numVerts;
	// dynamic mesh may have more capacity.
	u32 maxVerts;

	f32 *verts;
	f32 *uvs;
	f32 *normals;

	Vec3 *GetVert(i32 i)
	{
		u8 *mem = (u8 *)verts;
		mem += (sizeof(Vec3) * i);
		return (Vec3 *)mem;
	}

	void Clear()
	{
		this->numVerts = 0;
	}

	void AddTri(
		Vec3 v0, Vec3 v1, Vec3 v2,
		Vec2 uv0, Vec2 uv1, Vec2 uv2,
		Vec3 n0, Vec3 n1, Vec3 n2)
	{
		i32 i = this->numVerts;
		this->numVerts += 1;
		// step to
		i32 vertStride = sizeof(f32) * 3 * i;
		i32 uvStride = sizeof(f32) * 2 * i;
		Vec3 *vert = (Vec3 *)((u8 *)verts + vertStride);
		Vec2 *uv = (Vec2 *)((u8 *)uvs + uvStride);
		Vec3 *normal = (Vec3 *)((u8 *)normals + vertStride);
		vert[0] = v0;
		vert[1] = v1;
		vert[2] = v2;
		uv[0] = uv0;
		uv[1] = uv1;
		uv[2] = uv2;
		normal[0] = n0;
		normal[1] = n1;
		normal[2] = n2;
	}

	void AddVert(
		Vec3 vert,
		Vec2 uv,
		Vec3 normal)
	{
		if (this->numVerts >= this->maxVerts)
		{
			return;
		}
		i32 i = this->numVerts;
		this->numVerts += 1;
		// step to
		i32 vertStride = sizeof(Vec3) * i;
		i32 uvStride = sizeof(Vec2) * i;
		Vec3 *vertPtr = (Vec3 *)((u8 *)verts + vertStride);
		Vec2 *uvPtr = (Vec2 *)((u8 *)uvs + uvStride);
		Vec3 *normalPtr = (Vec3 *)((u8 *)normals + vertStride);
		vertPtr[0] = vert;
		uvPtr[0] = uv;
		normalPtr[0] = normal;
	}

	i32 MeasureBytes()
	{
		i32 bytes = 0;
		const i32 v3size = sizeof(f32) * 3;
		const i32 v2size = sizeof(f32) * 2;
		bytes += v3size * numVerts;
		bytes += v2size * numVerts;
		bytes += v3size * numVerts;
		return bytes;
	}

	i32 CopyData(ZRMeshData original)
	{
		if (original.numVerts > maxVerts)
		{
			printf("No space to copy mesh! %d verts have %d\n",
				   original.numVerts, maxVerts);
			return ZE_ERROR_NO_SPACE;
		}
		// no need to set max verts, we assume we have the capacity for it
		numVerts = original.numVerts;

		i32 numVertBytes = (sizeof(f32) * 3) * numVerts;
		i32 numUVSBytes = (sizeof(f32) * 2) * numVerts;
		printf("Copying %d verts (%d vert bytes, %d uv bytes, %d normal bytes)\n",
			   numVerts, numVertBytes, numUVSBytes, numVertBytes);
		// i32 numFloats = numVerts * 3;
		// for (i32 i = 0; i < numFloats; ++i)
		// {
		//     this->verts[i] = original.verts[i];
		// }
		ZE_Copy(this->verts, original.verts, numVertBytes);
		ZE_Copy(this->uvs, original.uvs, numUVSBytes);
		ZE_Copy(this->normals, original.normals, numVertBytes);

		// ZE_CompareMemory((u8 *)verts, (u8 *)(original.verts), );
		return ZE_ERROR_NONE;
	}

	void PrintVerts()
	{
		printf("--- %d of %d verts ---\n", numVerts, maxVerts);
		f32 *cursor = verts;
		for (u32 i = 0; i < numVerts; ++i)
		{
			printf("%d: %.3f, %.3f, %.3f\n", i, cursor[0], cursor[1], cursor[2]);
			cursor += 3;
		}
	}
};

struct ZRMeshAsset
{
	ZRAsset header;
	ZRMeshData data;
};

struct ZEGrid2D
{
	i32 bIsDirty;
	i32 width;
	i32 height;
};

struct ZEIntGrid2
{
	ZEGrid2D header;
	u32 handle;
	int* data;
};

struct ZRU16Texture
{
	ZEGrid2D header;
	u32 handle;
	u16* data;
};

struct ZRVec4Texture
{
	ZEGrid2D header;
	u32 handle;
	Vec4* data;
};

///////////////////////////////////////////////////////////
// Render scene data types
///////////////////////////////////////////////////////////

#define ZR_DRAWOBJ_TYPE_NONE 0
#define ZR_DRAWOBJ_TYPE_MESH 1
#define ZR_DRAWOBJ_TYPE_POINT_LIGHT 2
#define ZR_DRAWOBJ_TYPE_DIRECT_LIGHT 3
#define ZR_DRAWOBJ_TYPE_TEXT 4
#define ZR_DRAWOBJ_TYPE_BILLBOARD 5
#define ZR_DRAWOBJ_TYPE_PARTICLES 6
#define ZR_DRAWOBJ_TYPE_SPRITE 7
#define ZR_DRAWOBJ_TYPE_BOUNDING_BOX 8
#define ZR_DRAWOBJ_TYPE_QUAD 9
#define ZR_DRAWOBJ_TYPE_LINES 10

#define ZR_DRAWOBJ_STATUS_FREE 0
#define ZR_DRAWOBJ_STATUS_ASSIGNED 1
#define ZR_DRAWOBJ_STATUS_DELETED 2

struct ZRMeshObjData
{
	i32 meshId;
	i32 materialId;
	i32 billboard;
};

struct ZRBoundingBox
{
	AABB aabb;
	ColourU32 colour;
};

struct ZRDrawObjData
{
	i32 type;
	union
	{
		ZRMeshObjData model;
		ZRBoundingBox box;
		struct
		{
			u32 frameId;
		} sprite;
		struct
		{
			i32 bCastShadows;
			ColourU32 colour;
			f32 multiplier;
			f32 range;
		} pointLight;
		struct
		{
			i32 bCastShadows;
			ColourU32 colour;
			f32 multiplier;
			f32 range;
		} directLight;
		ZRQuad quad;
		struct
		{
			char *text;
			i32 length;
			i32 linesPerScreen;
			i32 charTextureId;
			ColourU32 colour;
			ColourU32 bgColour;
			i32 alignment;
		} text;
		struct
		{
			ZRLineVertex* verts;
			i32 numVerts;
			// verts is allocated as a fixed buffer
			i32 maxVerts;
			i32 bChained;
		} lines;
	};

	void SetAsMesh(i32 meshId, i32 materialId)
	{
		this->type = ZR_DRAWOBJ_TYPE_MESH;
		this->model.meshId = meshId;
		this->model.materialId = materialId;
	}

	void SetAsMeshFromData(ZRMeshObjData data)
	{
		this->type = ZR_DRAWOBJ_TYPE_MESH;
		this->model.meshId = data.meshId;
		this->model.materialId = data.materialId;
	}

	void SetAsBoundingBox(AABB aabb, ColourU32 colour)
	{
		this->type = ZR_DRAWOBJ_TYPE_BOUNDING_BOX;
		this->box.aabb = aabb;
		this->box.colour = colour;
	}

	void SetAsSprite(u32 spriteFrameId)
	{
		this->type = ZR_DRAWOBJ_TYPE_SPRITE;
		this->sprite.frameId = spriteFrameId;
	}

	void SetAsPointLight(ColourU32 colour, f32 multiplier, f32 radius)
	{
		this->type = ZR_DRAWOBJ_TYPE_POINT_LIGHT;
		this->pointLight.colour = colour;
		this->pointLight.multiplier = multiplier;
		this->pointLight.range = radius;
	}

	void SetAsDirectLight(ColourU32 colour, f32 multiplier, f32 radius)
	{
		this->type = ZR_DRAWOBJ_TYPE_DIRECT_LIGHT;
		this->directLight.colour = colour;
		this->directLight.multiplier = multiplier;
		this->directLight.range = radius;
	}

	void SetAsText(char *chars, i32 texId, ColourU32 colour, ColourU32 bgColour, i32 alignment)
	{
		this->type = ZR_DRAWOBJ_TYPE_TEXT;
		this->text.text = chars;
		this->text.length = ZStr_LenNoTerminator(chars) + 1;
		if (texId >= 0)
		{
			this->text.charTextureId = texId;
		}
		else
		{
			this->text.charTextureId = 0;
		}
		this->text.colour = colour;
		this->text.bgColour = bgColour;
		this->text.alignment = alignment;
	}
	
	void UpdateText(char* chars)
	{
		ZE_ASSERT(this->type == ZR_DRAWOBJ_TYPE_TEXT, "DrawObj is not text")
		this->text.text = chars;
		this->text.length = ZStr_LenNoTerminator(chars) + 1;
	}
};

struct ZRDrawObj
{
	Transform t;
	// For interpolation
	Vec3 prevPos;
	// an Id used to locate this object.
	zeHandle id;
	// userTag is for external game use and is not used by the engine itself.
	i32 userTag;
	// user data blob allocated at the same time as this object.
	// will be of a fixed size specified when the scene was created.
	void* userData;
	// hash of data union, used to identify objects which are
	// similar and could be batched
	u32 hash;
	ZRDrawObjData data;
	u32 CalcDataHash()
	{
		hash = ZE_Hash_djb2_Fixed((u8 *)&this->data, sizeof(ZRDrawObjData));
		return hash;
	}
};

#endif // ZENGINE_TYPES_H
