#ifndef ZE_MATH_FUNCTIONS_H
#define ZE_MATH_FUNCTIONS_H

#include "../ze_common.h"

#include <math.h>

//#define ArrayCount(array) (sizeof(array) / sizeof(array)[0]))

#define VEC_X 0
#define VEC_Y 1
#define VEC_Z 2
#define VEC_W 3

/* Matrix use
OpenGL uses column major, y/x matrices
/   0   4   8   12  \
|   1   5   9   13  |
|   2   6   10  14  |
\   3   7   11  15  /

*/

// Access Matrix Arrays
#define M3x3_X0 0
#define M3x3_X1 1
#define M3x3_X2 2

#define M3x3_Y0 3
#define M3x3_Y1 4
#define M3x3_Y2 5

#define M3x3_Z0 6
#define M3x3_Z1 7
#define M3x3_Z2 8

#define M4x4_X0 0
#define M4x4_X1 1
#define M4x4_X2 2
#define M4x4_X3 3

#define M4x4_Y0 4
#define M4x4_Y1 5
#define M4x4_Y2 6
#define M4x4_Y3 7

#define M4x4_Z0 8
#define M4x4_Z1 9
#define M4x4_Z2 10
#define M4x4_Z3 11

#define M4x4_W0 12
#define M4x4_W1 13
#define M4x4_W2 14
#define M4x4_W3 15

inline AABB AABB_FromCube(f32 size)
{
	f32 halfSize = size / 2.f;
	AABB r;
	r.min.x = -halfSize;
	r.min.y = -halfSize;
	r.min.z = -halfSize;

	r.max.x = halfSize;
	r.max.y = halfSize;
	r.max.z = halfSize;
	return r;
}

inline AABB AABB_Combine(AABB a, AABB b)
{
	AABB result;
	result.min.x = a.min.x < b.min.x ? a.min.x : b.min.x;
	result.min.y = a.min.y < b.min.y ? a.min.y : b.min.y;
	result.min.z = a.min.z < b.min.z ? a.min.z : b.min.z;

	result.max.x = a.max.x > b.max.x ? a.max.x : b.max.x;
	result.max.y = a.max.y > b.max.y ? a.max.y : b.max.y;
	result.max.z = a.max.z > b.max.z ? a.max.z : b.max.z;
	return result;
}

/**
 * Axis Aligned - No rotation!
 */
inline AABB AABB_LocalToWorld(Vec3 pos, AABB aabb)
{
	// AABB a = {};
	// a.min.x = pos.x + aabb.min.x;
	// a.min.y = pos.y + aabb.min.y;
	// a.min.z = pos.z + aabb.min.z;

	// a.max.x = pos.x + aabb.max.x;
	// a.max.y = pos.y + aabb.max.y;
	// a.max.z = pos.z + aabb.max.z;
	AABB a = aabb;
	a.min.x += pos.x;
	a.min.y += pos.y;
	a.min.z += pos.z;

	a.max.x += pos.x;
	a.max.y += pos.y;
	a.max.z += pos.z;
	return a;
}

inline Vec3 AABB_RandomInside(AABB aabb, f32 seedX, f32 seedY, f32 seedZ)
{
	return
	{
		aabb.min.x + aabb.Width() * seedX,
		aabb.min.y + aabb.Height() * seedY,
		aabb.min.z + aabb.Breadth() * seedZ
	};
}

//internal i32 g_z_inf = 0x7F800000;
//f32 Z_INFINITY;

//internal i32 g_z_nan = 0x7F800001;
f32 ZNaN();

// internal i32 ZE_STDRandI32();
// internal u8 ZE_STDRandU8();
// internal f32 ZE_STDRandf32();
// internal f32 ZE_STDRandomInRange(f32 min, f32 max);

internal void ZE_ClampF32(f32 *val, f32 min, f32 max)
{
	if (*val < min)
	{
		*val = min;
	}
	if (*val > max)
	{
		*val = max;
	}
}

internal void ZE_ClampI32(i32 *val, i32 min, i32 max)
{
	if (*val < min)
	{
		*val = min;
	}
	if (*val > max)
	{
		*val = max;
	}
}

internal f32 ZE_LerpF32(f32 start, f32 end, f32 lerp)
{
	//return start + lerp * (end - start);
	return start + ((end - start) * lerp);
}

// internal float ZE_LinearEase(
// 	f32 currentIteration,
// 	f32 startValue,
// 	f32 changeInValue,
// 	f32 totalIterations);
// internal f32 ZE_CapAngleDegrees(f32 angle);

/////////////////////////////////////////////////////////////////////////////
// VECTOR 2 OPERATIONS
/////////////////////////////////////////////////////////////////////////////
inline Vec2 Vec2_FromVec3(Vec3 v)
{
	return {v.x, v.y};
}

inline Vec2 Vec2_FromPoint2(Point2 p)
{
	return { (f32)p.x, (f32)p.y };
}

inline Vec2 Vec2_Add(Vec2 to, Vec2 amount)
{
	return { to.x + amount.x, to.y + amount.y };
}

inline Vec2 Vec2_Subtract(Vec2 from, Vec2 amount)
{
	return { from.x - amount.x, from.y - amount.y };
}

inline Vec2 Vec2_Mul(Vec2 v, f32 multiplier)
{
	return { v.x * multiplier, v.y * multiplier };
}

inline Vec2 Vec2_Divide(Vec2 v, f32 divider)
{
	return { v.x / divider, v.y / divider };
}

inline f32 Vec2_AngleTo(Vec2 from, Vec2 to)
{
	f32 vx = to.x - from.x;
	f32 vy = to.y - from.y;
	return atan2f(vy, vx);
}

/////////////////////////////////////////////////////////////////////////////
// VECTOR 3 OPERATIONS
/////////////////////////////////////////////////////////////////////////////

inline Vec4 ZE_Vec3ToVec4(Vec3 v3, f32 w)
{
	return {v3.x, v3.y, v3.z, w};
}

inline Vec3 Vec3_FromVec2(Vec2 v, f32 z)
{
	return {v.x, v.y, z};
}

internal Vec3 Vec3_FromVec4(Vec4 v)
{
	return {v.x, v.y, v.z};
}

internal i32 Vec3_IsZero(Vec3 *v)
{
	return (v->x == 0 && v->y == 0 && v->z == 0);
}

internal f32 Vec3_MagnitudeSqr(Vec3 *v)
{
	return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

internal f32 Vec3_Magnitude(Vec3 *v)
{
	return (f32)sqrt((f32)(v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

internal f32 Vec3_Magnitudef(f32 x, f32 y, f32 z)
{
	return (f32)sqrt((f32)(x * x) + (y * y) + (z * z));
}

inline void Vec3_AddTo(Vec3* v, Vec3 addition)
{
	v->x += addition.x;
	v->y += addition.y;
	v->z += addition.z;
}

inline Vec3 Vec3_Add(Vec3 a, Vec3 b)
{
	Vec3 c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return c;
}

inline Vec3 Vec3_Subtract(Vec3 a, Vec3 b)
{
	Vec3 c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return c;
}

inline void Vec3_MulF(Vec3* v, f32 val)
{
	v->x *= val;
	v->y *= val;
	v->z *= val;
}

internal Vec3 Vec3_Flipped(Vec3 v)
{
	return {-v.x, -v.y, -v.z};
}

internal void Vec3_Normalise(Vec3 *v)
{
	f32 vectorMagnitude = Vec3_Magnitude(v);
	if (vectorMagnitude == 0)
	{
		*v = {0, 0, 0};
		return;
	}
	v->x /= vectorMagnitude;
	v->y /= vectorMagnitude;
	v->z /= vectorMagnitude;
}

internal Vec3 Vec3_Normalised(Vec3 v)
{
	f32 vectorMagnitude = Vec3_Magnitude(&v);
	if (vectorMagnitude == 0)
	{
		return {0, 0, 0};
	}
	return {
		v.x /= vectorMagnitude,
		v.y /= vectorMagnitude,
		v.z /= vectorMagnitude};
}

internal Vec3 Vec3_Lerp(Vec3 start, Vec3 end, f32 lerp)
{
	return {
		ZE_LerpF32(start.x, end.x, lerp),
		ZE_LerpF32(start.y, end.y, lerp),
		ZE_LerpF32(start.z, end.z, lerp)};
}

internal Vec3 Vec3_GetNormal(Vec3 *v)
{
	Vec3 result = {};
	f32 vectorMagnitude = Vec3_Magnitude(v);
	if (vectorMagnitude == 0)
	{
		return result;
	}
	result.x = v->x / vectorMagnitude;
	result.y = v->y / vectorMagnitude;
	result.z = v->z / vectorMagnitude;
	return result;
}

internal void Vec3_SetMagnitude(Vec3 *v, f32 newMagnitude)
{
	Vec3_Normalise(v);
	v->x *= newMagnitude;
	v->y *= newMagnitude;
	v->z *= newMagnitude;
}

internal void Vec3_CapMagnitude(Vec3 *v, f32 min, f32 max)
{
	f32 minSqr = min * min;
	f32 maxSqr = max * max;
	f32 magSqr = Vec3_MagnitudeSqr(v);
	if (magSqr > maxSqr)
	{
		Vec3_SetMagnitude(v, max);
	}
	else if (magSqr < minSqr)
	{
		*v = {};
	}
}

internal f32 Vec3_Distance(Vec3 a, Vec3 b)
{
	return Vec3_Magnitudef(b.x - a.x, b.y - a.y, b.z - a.z);
}

internal i32 Point_Distance(Point2 a, Point2 b)
{
	i32 x = b.x - a.x;
	i32 y = b.y - a.y;

	return (i32)roundf((f32)sqrt((f32)(x * x) + (f32)(y * y)));
}

internal Vec3 Vec3_CrossProduct(Vec3 *a, Vec3 *b)
{
	return {
		(a->y * b->z) - (a->z * b->y),
		(a->x * b->y) - (a->y * b->x),
		(a->z * b->x) - (a->x * b->z)};
}

internal Vec3 Vec3_CrossProduct(Vec3 a, Vec3 b)
{
	return {
		(a.y * b.z) - (a.z * b.y),
		(a.x * b.y) - (a.y * b.x),
		(a.z * b.x) - (a.x * b.z)};
}

internal f32 Vec3_DotProduct(Vec3 *a, Vec3 *b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

internal f32 Vec3_DotProduct(Vec3 a, Vec3 b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

internal void Vec3_NormaliseOrForward(Vec3 *v)
{
	f32 mag = Vec3_Magnitude(v);
	if (mag == 0)
	{
		v->x = 0;
		v->y = 0;
		v->z = 1;
		return;
	}
	v->x = v->x /= mag;
	v->y = v->y /= mag;
	v->z = v->z /= mag;
}

internal void Vec3_MultiplyByM3x3(Vec3 *v, f32 *m)
{
	Vec3 r;
	r = {};
	r.x = (m[M3x3_X0] * v->x) + (m[M3x3_Y0] * v->y) + (m[M3x3_Z0] * v->z);
	r.y = (m[M3x3_X1] * v->x) + (m[M3x3_Y1] * v->y) + (m[M3x3_Z1] * v->z);
	r.z = (m[M3x3_X2] * v->x) + (m[M3x3_Y2] * v->y) + (m[M3x3_Z2] * v->z);
	*v = r;
	// return r;
}

internal Vec3 Vec3_MultiplyByM4x4(Vec3 *v, f32 *m)
{
	Vec3 r;
	f32 w = 1;
	r = {};
	r.x = (m[M4x4_X0] * v->x) + (m[M4x4_Y0] * v->y) + (m[M4x4_Z0] * v->z) + (m[M4x4_W0] * w);
	r.y = (m[M4x4_X1] * v->x) + (m[M4x4_Y1] * v->y) + (m[M4x4_Z1] * v->z) + (m[M4x4_W1] * w);
	r.z = (m[M4x4_X2] * v->x) + (m[M4x4_Y2] * v->y) + (m[M4x4_Z2] * v->z) + (m[M4x4_W2] * w);
	return r;
}

internal void Vec3_MultiplyArrayByM4x4(Vec3 *vecs, i32 numVecs, f32 *m)
{
	for (i32 i = 0; i < numVecs; ++i)
	{
		vecs[i] = Vec3_MultiplyByM4x4(&vecs[i], m);
	}
}

// returns 1 if vectors are different
internal i32 Vec3_AreDifferent(Vec3 *a, Vec3 *b, f32 epsilon)
{
	f32 diff;
	diff = ZAbsf((b->x - a->x));
	if (diff > epsilon)
	{
		return 1;
	}
	diff = ZAbsf((b->y - a->y));
	if (diff > epsilon)
	{
		return 1;
	}
	diff = ZAbsf((b->z - a->z));
	if (diff > epsilon)
	{
		return 1;
	}

	return 0;
}

internal void Vec3_ClearZeroes(Vec3 *v)
{
	if (v->x == 0)
	{
		v->x = 1;
	}
	if (v->y == 0)
	{
		v->y = 1;
	}
	if (v->z == 0)
	{
		v->z = 1;
	}
}

internal Vec3 Vec3_VectorMA(Vec3 start, f32 scale, Vec3 forward)
{
	Vec3 result;
	result.x = start.x + (forward.x * scale);
	result.y = start.y + (forward.y * scale);
	result.z = start.z + (forward.z * scale);
	return result;
}

internal Vec3 Vec3_EulerAngles(Vec3 v)
{
	Vec3 result = {};
	f32 flatMagnitude = Vec3_Magnitudef(v.x, 0, v.z);
	// yaw
	result.y = atan2f(v.x, v.z);
	// pitch
	result.x = -atan2f(v.y, flatMagnitude);
	return result;
}

internal Vec3 Vec3_EulerAnglesBetween(Vec3 a, Vec3 b)
{
	Vec3 result = {};
	Vec3 d;
	d.x = b.x - a.x;
	d.y = b.y - a.y;
	d.z = b.z - a.z;
	f32 flatMagnitude = Vec3_Magnitudef(d.x, 0, d.z);
	// yaw
	result.y = atan2f(d.x, d.z);
	// pitch
	result.x = atan2f(d.y, flatMagnitude);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// VECTOR 4 OPERATIONS
/////////////////////////////////////////////////////////////////////////////

internal Vec4 Vec4_FromVec3(Vec3 v, f32 w)
{
	return {v.x, v.y, v.z, w};
}

internal f32 Vec4_Magnitude(Vec4 *v)
{
	return (f32)sqrt((f32)(v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

internal void Vec4_Normalise(Vec4 *v)
{
	f32 vectorMagnitude = Vec4_Magnitude(v);
	v->x /= vectorMagnitude;
	v->y /= vectorMagnitude;
	v->z /= vectorMagnitude;
}

internal void Vec4_SetMagnitude(Vec4 *v, f32 newMagnitude)
{
	Vec4_Normalise(v);
	v->x = v->x * newMagnitude;
	v->y = v->y * newMagnitude;
	v->z = v->z * newMagnitude;
}

/////////////////////////////////////////////////////////////////////////////
// M3x3 OPERATIONS
/////////////////////////////////////////////////////////////////////////////

#define M3x3_CREATE(varName) \
	M3x3 varName##;          \
	M3x3_SetToIdentity(##varName.cells##);

inline void M3x3_SetToIdentity(f32 *m)
{
	m[M3x3_X0] = 1;
	m[M3x3_X1] = 0;
	m[M3x3_X2] = 0;

	m[M3x3_Y0] = 0;
	m[M3x3_Y1] = 1;
	m[M3x3_Y2] = 0;

	m[M3x3_Z0] = 0;
	m[M3x3_Z1] = 0;
	m[M3x3_Z2] = 1;
}

inline void M3x3_Multiply(f32 *a, f32 *b, f32 *result)
{
	/*
                0   3   6
                1   4   7 
                2   5   8
                |   |   | 
    0   3   6 -
    1   4   7 -
    2   5   8-

    result 3 = (0 x 3) + (3 x 4) + (6 x 5)
    result 4 = (1 x 3) + (4 x 4) + (7 x 5)
    result 5 = (2 x 3) + (5 x 4) + (8 x 5)

    result 6 = (0 x 6) + (3 x 7) + (6 x 8)
    result 7 = (1 x 6) + (4 x 7) + (7 x 8)
    result 8 = (2 x 6) + (5 x 7) + (8 x 8)

    */
	f32 r[9];
	r[0] = (a[0] * b[0]) + (a[3] * b[1]) + (a[6] * b[2]);
	r[1] = (a[1] * b[0]) + (a[4] * b[1]) + (a[7] * b[2]);
	r[2] = (a[2] * b[0]) + (a[5] * b[1]) + (a[8] * b[2]);

	r[3] = (a[0] * b[3]) + (a[3] * b[4]) + (a[6] * b[5]);
	r[4] = (a[1] * b[3]) + (a[4] * b[4]) + (a[7] * b[5]);
	r[5] = (a[2] * b[3]) + (a[5] * b[4]) + (a[8] * b[5]);

	r[6] = (a[0] * b[6]) + (a[3] * b[7]) + (a[6] * b[8]);
	r[7] = (a[1] * b[6]) + (a[4] * b[7]) + (a[7] * b[8]);
	r[8] = (a[2] * b[6]) + (a[5] * b[7]) + (a[8] * b[8]);

	result[0] = r[0];
	result[1] = r[1];
	result[2] = r[2];
	result[3] = r[3];
	result[4] = r[4];
	result[5] = r[5];
	result[6] = r[6];
	result[7] = r[7];
	result[8] = r[8];
}
inline void M3x3_BuildRotateByAxis(f32 *m, f32 radians, f32 x, f32 y, f32 z)
{
	f32 c = cosf(radians);
	f32 s = sinf(radians);
	m[0] = (x * x) * (1 - c) + (c);
	m[1] = (y * x) * (1 - c) + (z * s);
	m[2] = (x * z) * (1 - c) - (y * s);

	m[3] = (x * y) * (1 - c) - (z * s);
	m[4] = (y * y) * (1 - c) + (c);
	m[5] = (y * z) * (1 - c) + (x * s);

	m[6] = (x * z) * (1 - c) + (y * s);
	m[7] = (y * z) * (1 - c) - (x * s);
	m[8] = (z * z) * (1 - c) + (c);
}
// Rotate m radians degrees around the axis x/y/z
inline void M3x3_RotateByAxis(f32 *m, f32 radians, f32 x, f32 y, f32 z)
{
	f32 temp[16];
	M3x3_BuildRotateByAxis(temp, radians, x, y, z);
	M3x3_Multiply(m, temp, m);
}
internal void M3x3_SetX(f32 *m, f32 x0, f32 x1, f32 x2);
internal void M3x3_SetY(f32 *m, f32 y0, f32 y1, f32 y2);
internal void M3x3_SetZ(f32 *m, f32 z0, f32 z1, f32 z2);
internal void M3x3_Copy(f32 *src, f32 *tar);

internal void M3x3_RotateX(f32 *m, f32 radiansX)
{
	M3x3 rotM = {};
	rotM.xAxisX = 1;
	rotM.yAxisY = (f32)cos(radiansX);
	rotM.yAxisZ = (f32)sin(radiansX);
	rotM.zAxisY = (f32)-sin(radiansX);
	rotM.zAxisZ = (f32)cos(radiansX);
	M3x3_Multiply(m, rotM.cells, m);
}

internal void M3x3_RotateY(f32 *m, f32 radiansY)
{
	M3x3 rotM = {};
	rotM.yAxisY = 1;
	rotM.xAxisX = (f32)cos(radiansY);
	rotM.xAxisZ = (f32)-sin(radiansY);
	rotM.zAxisX = (f32)sin(radiansY);
	rotM.zAxisZ = (f32)cos(radiansY);
	M3x3_Multiply(m, rotM.cells, m);
}

internal void M3x3_RotateZ(f32 *m, f32 radiansZ)
{
	M3x3 rotM = {};
	rotM.zAxisZ = 1;
	rotM.xAxisX = (f32)cos(radiansZ);
	rotM.xAxisY = (f32)sin(radiansZ);
	rotM.yAxisX = (f32)-sin(radiansZ);
	rotM.yAxisY = (f32)cos(radiansZ);
	M3x3_Multiply(m, rotM.cells, m);
}

// internal f32 M3x3_GetAngleX(f32 *m);
// internal f32 M3x3_GetAngleY(f32 *m);
// internal f32 M3x3_GetAngleZ(f32 *m);

internal Vec3 M3x3_GetEulerAnglesRadians(f32 *m)
{
	Vec3 result;
	/*result.x = (f32)-asinf(m[9]);
    result.y = (f32)atan2(m[8], m[10]);
    result.z = (f32)atan2(m[1], m[5]);*/
	result.x = (f32)-asinf(m[M3x3_Z1]);
	result.y = (f32)atan2(m[M3x3_Z0], m[M3x3_Z2]);
	result.z = (f32)atan2(m[M3x3_X1], m[M3x3_Y1]);
	//result.w = 1;
	return result;
}

// internal Vec3 M3x3_GetEulerAnglesDegrees(f32 *m);
// internal void M3x3_SetEulerAnglesByRadians(f32 *m, f32 roll, f32 pitch, f32 yaw);

internal void M3x3_CopyFromM4x4(f32 *m3x3, f32 *m4x4)
{
	m3x3[M3x3_X0] = m4x4[M4x4_X0];
	m3x3[M3x3_X1] = m4x4[M4x4_X1];
	m3x3[M3x3_X2] = m4x4[M4x4_X2];

	m3x3[M3x3_Y0] = m4x4[M4x4_Y0];
	m3x3[M3x3_Y1] = m4x4[M4x4_Y1];
	m3x3[M3x3_Y2] = m4x4[M4x4_Y2];

	m3x3[M3x3_Z0] = m4x4[M4x4_Z0];
	m3x3[M3x3_Z1] = m4x4[M4x4_Z1];
	m3x3[M3x3_Z2] = m4x4[M4x4_Z2];
}

/////////////////////////////////////////////////////////////////////////////
// M4x4 OPERATIONS
/////////////////////////////////////////////////////////////////////////////

#define M4x4_CREATE(varName) \
	M4x4 varName##;          \
	M4x4_SetToIdentity(##varName.cells##);

inline void M4x4_SetToIdentity(f32 *m)
{
	m[M4x4_X0] = 1;
	m[M4x4_X1] = 0;
	m[M4x4_X2] = 0;
	m[M4x4_X3] = 0;

	m[M4x4_Y0] = 0;
	m[M4x4_Y1] = 1;
	m[M4x4_Y2] = 0;
	m[M4x4_Y3] = 0;

	m[M4x4_Z0] = 0;
	m[M4x4_Z1] = 0;
	m[M4x4_Z2] = 1;
	m[M4x4_Z3] = 0;

	m[M4x4_W0] = 0;
	m[M4x4_W1] = 0;
	m[M4x4_W2] = 0;
	m[M4x4_W3] = 1;
}

inline void M4x4_ClearRotation(f32 *m)
{
	m[M4x4_X0] = 1;
	m[M4x4_X1] = 0;
	m[M4x4_X2] = 0;

	m[M4x4_Y0] = 0;
	m[M4x4_Y1] = 1;
	m[M4x4_Y2] = 0;

	m[M4x4_Z0] = 0;
	m[M4x4_Z1] = 0;
	m[M4x4_Z2] = 1;
}

inline void M4x4_Multiply(f32 *m0, f32 *m1, f32 *result)
{
	/*
                    0   4   8   12
                    1   5   9   13
                    2   6   10  14
                    3   7   11  15
                    |   |   |   |
    0   4   8   12-
    1   5   9   13-
    2   6   10  14-
    3   7   11  15-
    */

	f32 r[16];

	r[0] = (m0[0] * m1[0]) + (m0[4] * m1[1]) + (m0[8] * m1[2]) + (m0[12] * m1[3]);
	r[1] = (m0[1] * m1[0]) + (m0[5] * m1[1]) + (m0[9] * m1[2]) + (m0[13] * m1[3]);
	r[2] = (m0[2] * m1[0]) + (m0[6] * m1[1]) + (m0[10] * m1[2]) + (m0[14] * m1[3]);
	r[3] = (m0[3] * m1[0]) + (m0[7] * m1[1]) + (m0[11] * m1[2]) + (m0[15] * m1[3]);

	r[4] = (m0[0] * m1[4]) + (m0[4] * m1[5]) + (m0[8] * m1[6]) + (m0[12] * m1[7]);
	r[5] = (m0[1] * m1[4]) + (m0[5] * m1[5]) + (m0[9] * m1[6]) + (m0[13] * m1[7]);
	r[6] = (m0[2] * m1[4]) + (m0[6] * m1[5]) + (m0[10] * m1[6]) + (m0[14] * m1[7]);
	r[7] = (m0[3] * m1[4]) + (m0[7] * m1[5]) + (m0[11] * m1[6]) + (m0[15] * m1[7]);

	r[8] = (m0[0] * m1[8]) + (m0[4] * m1[9]) + (m0[8] * m1[10]) + (m0[12] * m1[11]);
	r[9] = (m0[1] * m1[8]) + (m0[5] * m1[9]) + (m0[9] * m1[10]) + (m0[13] * m1[11]);
	r[10] = (m0[2] * m1[8]) + (m0[6] * m1[9]) + (m0[10] * m1[10]) + (m0[14] * m1[11]);
	r[11] = (m0[3] * m1[8]) + (m0[7] * m1[9]) + (m0[11] * m1[10]) + (m0[15] * m1[11]);

	r[12] = (m0[0] * m1[12]) + (m0[4] * m1[13]) + (m0[8] * m1[14]) + (m0[12] * m1[15]);
	r[13] = (m0[1] * m1[12]) + (m0[5] * m1[13]) + (m0[9] * m1[14]) + (m0[13] * m1[15]);
	r[14] = (m0[2] * m1[12]) + (m0[6] * m1[13]) + (m0[10] * m1[14]) + (m0[14] * m1[15]);
	r[15] = (m0[3] * m1[12]) + (m0[7] * m1[13]) + (m0[11] * m1[14]) + (m0[15] * m1[15]);

	// copy result
	result[0] = r[0];
	result[1] = r[1];
	result[2] = r[2];
	result[3] = r[3];
	result[4] = r[4];
	result[5] = r[5];
	result[6] = r[6];
	result[7] = r[7];
	result[8] = r[8];
	result[9] = r[9];
	result[10] = r[10];
	result[11] = r[11];
	result[12] = r[12];
	result[13] = r[13];
	result[14] = r[14];
	result[15] = r[15];
}
// Rotate m radians degrees around the axis x/y/z
inline void M4x4_BuildRotateByAxis(f32 *m, f32 radians, f32 x, f32 y, f32 z)
{
	f32 c = cosf(radians);
	f32 s = sinf(radians);
	m[0] = (x * x) * (1 - c) + (c);
	m[1] = (y * x) * (1 - c) + (z * s);
	m[2] = (x * z) * (1 - c) - (y * s);
	m[3] = 0;

	m[4] = (x * y) * (1 - c) - (z * s);
	m[5] = (y * y) * (1 - c) + (c);
	m[6] = (y * z) * (1 - c) + (x * s);
	m[7] = 0;

	m[8] = (x * z) * (1 - c) + (y * s);
	m[9] = (y * z) * (1 - c) - (x * s);
	m[10] = (z * z) * (1 - c) + (c);
	m[11] = 0;

	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}

inline void M4x4_RotateByAxis(f32 *m, f32 radians, f32 x, f32 y, f32 z)
{
	f32 temp[16];
	M4x4_BuildRotateByAxis(temp, radians, x, y, z);
	M4x4_Multiply(m, temp, m);
}

inline void M4x4_BuildTranslation(f32 *m, f32 x, f32 y, f32 z)
{
	m[0] = 1;
	m[1] = 0;
	m[2] = 0;
	m[3] = 0;

	m[4] = 0;
	m[5] = 1;
	m[6] = 0;
	m[7] = 0;

	m[8] = 0;
	m[9] = 0;
	m[10] = 1;
	m[11] = 0;

	m[12] = x;
	m[13] = y;
	m[14] = z;
	m[15] = 1;
}
inline void M4x4_Translate(f32 *m, f32 x, f32 y, f32 z)
{
	f32 temp[16];
	M4x4_BuildTranslation(temp, x, y, z);
	M4x4_Multiply(m, temp, m);
}

inline void M4x4_BuildScale(f32 *m, f32 x, f32 y, f32 z)
{
	m[M4x4_X0] = x;
	m[M4x4_X1] = 0;
	m[M4x4_X2] = 0;
	m[M4x4_X3] = 0;

	m[M4x4_Y0] = 0;
	m[M4x4_Y1] = y;
	m[M4x4_Y2] = 0;
	m[M4x4_Y3] = 0;

	m[M4x4_Z0] = 0;
	m[M4x4_Z1] = 0;
	m[M4x4_Z2] = z;
	m[M4x4_Z3] = 0;

	m[M4x4_W0] = 0;
	m[M4x4_W1] = 0;
	m[M4x4_W2] = 0;
	m[M4x4_W3] = 1;
}

inline void M4x4_ApplyScale(f32 *m, f32 x, f32 y, f32 z)
{
	f32 temp[16];
	M4x4_BuildScale(temp, x, y, z);
	M4x4_Multiply(m, temp, m);
}

inline Vec3 M4x4_GetScale(f32 *m)
{
	Vec3 scale;
	scale.x = Vec3_Magnitudef(m[M4x4_X0], m[M4x4_X1], m[M4x4_X2]);
	scale.y = Vec3_Magnitudef(m[M4x4_Y0], m[M4x4_Y1], m[M4x4_Y2]);
	scale.z = Vec3_Magnitudef(m[M4x4_Z0], m[M4x4_Z1], m[M4x4_Z2]);
	return scale;
}

// Exact compare, no epsilon atm
inline i32 M4x4_Equals(f32 *a, f32 *b)
{
	if (a[0] != b[0])
	{
		return NO;
	}
	if (a[1] != b[1])
	{
		return NO;
	}
	if (a[2] != b[2])
	{
		return NO;
	}
	if (a[3] != b[3])
	{
		return NO;
	}
	if (a[4] != b[4])
	{
		return NO;
	}
	if (a[5] != b[5])
	{
		return NO;
	}
	if (a[6] != b[6])
	{
		return NO;
	}
	if (a[7] != b[7])
	{
		return NO;
	}
	if (a[8] != b[8])
	{
		return NO;
	}
	if (a[9] != b[9])
	{
		return NO;
	}
	if (a[10] != b[10])
	{
		return NO;
	}
	if (a[11] != b[11])
	{
		return NO;
	}
	if (a[12] != b[12])
	{
		return NO;
	}
	if (a[13] != b[13])
	{
		return NO;
	}
	if (a[14] != b[14])
	{
		return NO;
	}
	if (a[15] != b[15])
	{
		return NO;
	}
	return YES;
}

internal i32 M4x4_HasZeroPosition(f32 *m)
{
	return (m[M4x4_W0] == 0 && m[M4x4_W1] == 0 && m[M4x4_W2] == 0);
}

////////////////////////////////////////////////////////////////////
// Projection
////////////////////////////////////////////////////////////////////

internal void M4x4_SetProjection(f32 *m, f32 prjNear, f32 prjFar, f32 prjLeft, f32 prjRight, f32 prjTop, f32 prjBottom)
{
	m[0] = (2 * prjNear) / (prjRight - prjLeft);
	m[4] = 0;
	m[8] = (prjRight + prjLeft) / (prjLeft - prjRight);
	m[12] = 0;

	m[1] = 0;
	m[5] = (2 * prjNear) / (prjTop - prjBottom);
	m[9] = (prjTop + prjBottom) / (prjTop - prjBottom);
	m[13] = 0;

	m[2] = 0;
	m[6] = 0;
	m[10] = -(prjFar + prjNear) / (prjFar - prjNear);
	m[14] = (-2 * prjFar * prjNear) / (prjFar - prjNear);

	m[3] = 0;
	m[7] = 0;
	m[11] = -1;
	m[15] = 0;
}

internal void M4x4_SetOrthoProjection(f32 *m, f32 left, f32 right, f32 top, f32 bottom, f32 prjNear, f32 prjFar)
{
#if 1
	M4x4_SetToIdentity(m);
	m[0] = 2 / (right - left);
	m[5] = 2 / (top - bottom);
	m[10] = -2 / (prjFar - prjNear);

	m[12] = -(right + left) / (right - left);
	m[13] = -(top + bottom) / (top - bottom);
	m[14] = -(prjFar + prjNear) / (prjFar - prjNear);
	m[15] = 1;
#endif
#if 0
    m[0] = 2; 
    m[5] = 2;
    m[10] = -0.22f;

    m[14] = -1.22f;
    m[15] = 1;
#endif
}

inline void ZE_SetupOrthoProjection(f32 *m, f32 size, f32 aspectRatio)
{
	//M4x4_SetOrthoProjection(m, -1, 1, 1, -1, 0.1f, 20.f);
	//float size = 40;
	M4x4_SetOrthoProjection(m,
							-size * aspectRatio,
							size * aspectRatio,
							size,
							-size,
							0.1f, 60.f);
}

inline void ZE_Setup3DProjection(
	f32 *m4x4,
	i32 fov,
	f32 prjScaleFactor,
	f32 prjNear,
	f32 prjFar,
	f32 aspectRatio)
{
	if (fov <= 0)
	{
		fov = 90;
	}
	M4x4_SetToIdentity(m4x4);

	f32 prjLeft = -prjScaleFactor * aspectRatio;
	f32 prjRight = prjScaleFactor * aspectRatio;
	f32 prjTop = prjScaleFactor;
	f32 prjBottom = -prjScaleFactor;

	M4x4_SetProjection(
		m4x4, prjNear, prjFar, prjLeft, prjRight, prjTop, prjBottom);
}

inline void ZE_SetupDefault3DProjection(
	f32 *m4x4, f32 aspectRatio)
{
	//ZE_Setup3DProjection(m4x4, 90, 0.5f, 1.0f, 1000.0f, aspectRatio);
	ZE_Setup3DProjection(m4x4, 90, 0.07f, 0.1f, 1000.0f, aspectRatio);
}

////////////////////////////////////////////////////////////////////
// convert homogeneous (-1 to 1) coords to 0 to 1 uv coords
// (for shadow map sampling)
////////////////////////////////////////////////////////////////////
inline void M4x4_HomogeneousToUV(f32 *m)
{
	m[0] = 0.5f;
	m[1] = 0;
	m[2] = 0;
	m[3] = 0;

	m[4] = 0;
	m[5] = 0.5f;
	m[6] = 0;
	m[7] = 0;

	m[8] = 0;
	m[9] = 0;
	m[10] = 0.5f;
	m[11] = 0;

	m[12] = 0.5f;
	m[13] = 0.5f;
	m[14] = 0.5f;
	m[15] = 1;
}

// extern "C" void M4x4_InvertRotation(f32 *m);
// void M4x4_SetX(f32 *m, f32 x0, f32 x1, f32 x2, f32 x3);
// void M4x4_SetY(f32 *m, f32 y0, f32 y1, f32 y2, f32 y3);
// void M4x4_SetZ(f32 *m, f32 z0, f32 z1, f32 z2, f32 z3);
// void M4x4_SetW(f32 *m, f32 w0, f32 w1, f32 w2, f32 w3);
// void M4x4_Multiply(f32 *m0, f32 *m1, f32 *result);
// void M4x4_Copy(f32 *src, f32 *tar);
// void M4x4_RotateX(f32 *m, f32 radiansX);
// void M4x4_RotateY(f32 *m, f32 radiansY);

internal void M4x4_RotateY(f32 *m, f32 radiansY)
{
	M4x4 rotM = {};
	rotM.yAxisY = 1;
	rotM.posW = 1;
	rotM.xAxisX = (f32)cos(radiansY);
	rotM.xAxisZ = (f32)-sin(radiansY);
	rotM.zAxisX = (f32)sin(radiansY);
	rotM.zAxisZ = (f32)cos(radiansY);
	M4x4_Multiply(m, rotM.cells, m);
}

// void M4x4_RotateZ(f32 *m, f32 radiansY);
// f32 M4x4_GetAngleX(f32 *m);
// f32 M4x4_GetAngleY(f32 *m);
// f32 M4x4_GetAngleZ(f32 *m);
// void M4x4_SetPosition(f32 *m, f32 x, f32 y, f32 z);
// Vec4 M4x4_GetPosition(f32 *m);

inline Vec3 M4x4_GetEulerAnglesRadians(f32 *m)
{
	Vec3 result;
	result.x = (f32)-asinf(m[M4x4_Z1]);
	result.y = (f32)atan2(m[M4x4_Z0], m[M4x4_Z2]);
	result.z = (f32)atan2(m[M4x4_X1], m[M4x4_Y1]);
	return result;
}

// Vec3 M4x4_GetEulerAnglesDegrees(f32 *m);
// void M4x4_SetEulerAnglesByRadians(f32 *m, f32 roll, f32 pitch, f32 yaw);

// void M4x4_ApplyScale(f32 *m, f32 x, f32 y, f32 z);

inline void M4x4_SetScale(f32 *m, f32 x, f32 y, f32 z)
{
	M4x4 *mat = (M4x4 *)m;
	Vec4_SetMagnitude(&mat->xAxis, x);
	Vec4_SetMagnitude(&mat->yAxis, y);
	Vec4_SetMagnitude(&mat->zAxis, z);
}

// void M4x4_SetToScaling(f32 *m, f32 x, f32 y, f32 z);
// void M4x4_SetToTranslation(f32 *m, f32 x, f32 y, f32 z);

// void M4x4_Invert(f32 *src);
// void M4x4_ClearPosition(f32 *src);
// void M4x4_ClearRotation(f32 *src);

/*
// TODO: Change to using modulo or something...?
would like to use
angle = (angle % 360.0f);
if (angle < 0)
{ angle = -angle; }
but modulo only works for ints :(
modf uses floats though
*/
internal f32 ZE_CapAngleDegrees(f32 angle)
{
	u32 loopCount = 0; // everytime I think I don't need to do this...
	while (angle > 360)
	{
		angle -= 360;
		loopCount++;
		ZE_ASSERT(loopCount < 99999, "Loop ran away");
	}
	loopCount = 0;
	while (angle < 0)
	{
		angle += 360;
		loopCount++;
		ZE_ASSERT(loopCount < 99999, "Loop ran away");
	}
	return angle;
}

/////////////////////////////////////////////////////////////////////
// Position
/////////////////////////////////////////////////////////////////////

#define TRANSFORM_CREATE(varName) \
	Transform varName##;          \
	Transform_SetToIdentity(&##varName##);

extern "C" static void Transform_SetToIdentity(Transform *t)
{
	*t = {};
	t->pos = {0, 0, 0};
	t->scale = {1, 1, 1};
	M3x3_SetToIdentity(t->rotation.cells);
}

/////////////////////////////////////////////////////////////////////
// Conversion
/////////////////////////////////////////////////////////////////////

/**
 * Do NOT use this function to create a view matrix! Use Transform_ToViewMatrix
 */
extern "C" internal void Transform_ToM4x4(Transform *t, M4x4 *result)
{
	M4x4_SetToIdentity(result->cells);
	result->x0 = t->rotation.x0;
	result->x1 = t->rotation.x1;
	result->x2 = t->rotation.x2;

	result->y0 = t->rotation.y0;
	result->y1 = t->rotation.y1;
	result->y2 = t->rotation.y2;

	result->z0 = t->rotation.z0;
	result->z1 = t->rotation.z1;
	result->z2 = t->rotation.z2;

	result->w0 = t->pos.x;
	result->w1 = t->pos.y;
	result->w2 = t->pos.z;
	result->w3 = 1;

	Vec4_SetMagnitude(&result->xAxis, t->scale.x);
	Vec4_SetMagnitude(&result->yAxis, t->scale.y);
	Vec4_SetMagnitude(&result->zAxis, t->scale.z);
}

extern "C" internal void Transform_ToM4x4NoScale(Transform *t, M4x4 *result)
{
	M4x4_SetToIdentity(result->cells);
	result->x0 = t->rotation.x0;
	result->x1 = t->rotation.x1;
	result->x2 = t->rotation.x2;

	result->y0 = t->rotation.y0;
	result->y1 = t->rotation.y1;
	result->y2 = t->rotation.y2;

	result->z0 = t->rotation.z0;
	result->z1 = t->rotation.z1;
	result->z2 = t->rotation.z2;

	result->w0 = t->pos.x;
	result->w1 = t->pos.y;
	result->w2 = t->pos.z;
	result->w3 = 1;
}

inline void Transform_FromM4x4NoScale(Transform *t, f32 *m4x4)
{
	t->pos.x = m4x4[M4x4_W0];
	t->pos.y = m4x4[M4x4_W1];
	t->pos.z = m4x4[M4x4_W2];
	M3x3_CopyFromM4x4(t->rotation.cells, m4x4);
}

/**
Use this function NOT Transform_ToM4x4 to create a view
Matrix for rendering.
Resulting camera's forward will be looking into negative Z
if rotation is set to Identity
*/
inline void Transform_ToViewMatrix(Transform *camT, M4x4 *view)
{
	// View
	M4x4_SetToIdentity(view->cells);
	Vec3 camEuler = M3x3_GetEulerAnglesRadians(camT->rotation.cells);
	M4x4_RotateByAxis(view->cells, -camEuler.z, 0, 0, 1);
	M4x4_RotateByAxis(view->cells, -camEuler.x, 1, 0, 0);
	M4x4_RotateByAxis(view->cells, -camEuler.y, 0, 1, 0);
	// inverse camera translation
	M4x4_Translate(view->cells, -camT->pos.x, -camT->pos.y, -camT->pos.z);
}

/////////////////////////////////////////////////////////////////////
// Scale
/////////////////////////////////////////////////////////////////////
internal void Transform_SetScaleF(Transform *t, f32 scaleX, f32 scaleY, f32 scaleZ)
{
	t->scale.x = scaleX;
	t->scale.y = scaleY;
	t->scale.z = scaleZ;
}

// Avoid zero scales
internal void Transform_SetScaleSafe(Transform *t, Vec3 scale)
{
	if (scale.x != 0 && scale.y != 0 && scale.z != 0)
	{
		t->scale = scale;
	}
	else
	{
		t->scale = {1, 1, 1};
	}
}

/////////////////////////////////////////////////////////////////////
// Rotation
/////////////////////////////////////////////////////////////////////
internal void Transform_SetRotationDegrees(Transform *t, f32 degreesX, f32 degreesY, f32 degreesZ)
{
	M3x3_SetToIdentity(t->rotation.cells);
	M3x3_RotateZ(t->rotation.cells, degreesZ * DEG2RAD);
	M3x3_RotateY(t->rotation.cells, degreesY * DEG2RAD);
	M3x3_RotateX(t->rotation.cells, degreesX * DEG2RAD);
}

internal void Transform_SetRotation(Transform *t, f32 radiansX, f32 radiansY, f32 radiansZ)
{
	M3x3_SetToIdentity(t->rotation.cells);
	M3x3_RotateZ(t->rotation.cells, radiansZ);
	M3x3_RotateY(t->rotation.cells, radiansY);
	M3x3_RotateX(t->rotation.cells, radiansX);
}

internal void Transform_ApplyChain(Transform *target, Transform **links, i32 numLinks)
{
	ZE_ASSERT(target != NULL, "Target transform is null")
	ZE_ASSERT(links != NULL, "Transform links array is null")
	ZE_ASSERT(numLinks > 0, "Cannot chain 0 transforms")
	if (numLinks == 1)
	{
		*target = *links[0];
		return;
	}
	M4x4_CREATE(result)
		M4x4_CREATE(other) for (i32 i = 0; i < numLinks; ++i)
	{
		Transform_ToM4x4NoScale(links[i], &other);
		M4x4_Multiply(result.cells, other.cells, result.cells);
	}
	Transform_FromM4x4NoScale(target, result.cells);
}

static void Transform_Printf(Transform *t)
{
	printf("Pos %.3f, %.3f, %.3f\n", t->pos.x, t->pos.y, t->pos.z);
	printf("Rot:\n%.3f, %.3f, %.3f\n", t->rotation.x0, t->rotation.y0, t->rotation.z0);
	printf("%.3f, %.3f, %.3f\n", t->rotation.x1, t->rotation.y1, t->rotation.z1);
	printf("%.3f, %.3f, %.3f\n", t->rotation.x2, t->rotation.y2, t->rotation.z2);
}

#endif // ZE_MATH_FUNCTIONS_H