#ifndef ZE_MATH_TYPES_H
#define ZE_MATH_TYPES_H

#include "../ze_common.h"

/////////////////////////////////////////////////////////////////////////////
// Vector
/////////////////////////////////////////////////////////////////////////////
struct Point2
{
	union
	{
		struct
		{
			i32 x, y;
		};
		i32 parts[2];
	};
};

struct Point3
{
	union
	{
		struct
		{
			i32 x, y, z;
		};
		i32 parts[3];
	};
};

struct Vec2
{
	union
	{
		struct
		{
			f32 x, y;
		};
		f32 parts[2];
	};
};

struct Vec3
{
	union
	{
		struct
		{
			f32 x, y, z;
		};
		f32 parts[3];
	};
	// f32 x, y, z, w;
	// // overload array operator to return a pointer to x + index
	// f32 &operator[](int index) { return ((&x)[index]); }
};

const Vec3 vec3_zero = {};

struct Vec4
{
	union
	{
		struct
		{
			f32 x, y, z, w;
		};
		f32 e[4];
	};
	// f32 x, y, z, w;
	// // overload array operator to return a pointer to x + index
	// f32 &operator[](int index) { return ((&x)[index]); }
};

struct AABB
{
	Vec3 min;
	Vec3 max;

	f32 Width() { return max.x - min.x; }
	f32 Height() { return max.y - min.y; }
	f32 Breadth() { return max.z - min.z; }

	f32 HalfWidth() { return (max.x - min.x) / 2.f; }
	f32 HalfHeight() { return (max.y - min.y) / 2.f; }
	f32 HalfBreadth() { return (max.z - min.z) / 2.f; }

	f32 CentreX() { return min.x + ((max.x - min.x) / 2.f); }
	f32 CentreY() { return min.y + ((max.y - min.y) / 2.f); }
	f32 CentreZ() { return min.z + ((max.z - min.z) / 2.f); }

	f32 Volume()
	{
		return (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
	}
};

/////////////////////////////////////////////////////////////////////////////
// MATRIX 3x3
/////////////////////////////////////////////////////////////////////////////
struct M3x3
{
	union
	{
		/* Careful! Column order!
            X   Y   Z   W
            0   3   6
            1   4   7
            2   5   8
            */
		struct
		{
			f32
				x0,
				x1, x2,
				y0, y1, y2,
				z0, z1, z2;
		};
		struct
		{
			f32
				// xAxisX, yAxisX, zAxisX, posX,
				// xAxisY, yAxisY, zAxisY, posY,
				// xAxisZ, yAxisZ, zAxisZ, posZ,
				// xAxisW, yAxisW, zAxisW, posW;

				xAxisX,
				xAxisY, xAxisZ,
				yAxisX, yAxisY, yAxisZ,
				zAxisX, zAxisY, zAxisZ;
		};
		struct
		{
			Vec3 xAxis;
			Vec3 yAxis;
			Vec3 zAxis;
		};
		f32 cells[9];
	};
};

static_assert(sizeof(M3x3) == 36, "M4x4 is not 36 bytes!");

/////////////////////////////////////////////////////////////////////////////
// MATRIX 4x4
/////////////////////////////////////////////////////////////////////////////

/**
 * Should pack together to 64 bytes.
 */
struct M4x4
{
	union
	{
		/* Careful! Column order!
            X   Y   Z   W
            0   4   8   12
            1   5   9   13 
            2   6   10  14
            3   7   11  15
            */
		struct
		{
			f32
				x0,
				x1, x2, x3,
				y0, y1, y2, y3,
				z0, z1, z2, z3,
				w0, w1, w2, w3;
		};
		struct
		{
			f32
				// xAxisX, yAxisX, zAxisX, posX,
				// xAxisY, yAxisY, zAxisY, posY,
				// xAxisZ, yAxisZ, zAxisZ, posZ,
				// xAxisW, yAxisW, zAxisW, posW;

				xAxisX,
				xAxisY, xAxisZ, xAxisW,
				yAxisX, yAxisY, yAxisZ, yAxisW,
				zAxisX, zAxisY, zAxisZ, zAxisW,
				posX, posY, posZ, posW;
		};
		struct
		{
			Vec4 xAxis;
			Vec4 yAxis;
			Vec4 zAxis;
			Vec4 wAxis;
		};
		f32 cells[16];
	};
};

static_assert(sizeof(M4x4) == 64, "M4x4 is not 64 bytes!");

#define M3X3_MUL(ptrM3x4a, ptrM3x3b, ptrM3x3result) M3x3_Multiply(ptrM3x4a.cells, ptrM3x3b.cells, ptrM3x3result.cells);

struct Transform
{
	Vec3 pos;
	M3x3 rotation;
	Vec3 scale;
};

#endif // ZE_MATH_TYPES_H