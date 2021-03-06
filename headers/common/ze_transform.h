#ifndef ZE_TRANSFORM_H
#define ZE_TRANSFORM_H

#include "ze_common.h"

#define TRANSFORM_CREATE(varName) Transform varName##; Transform_SetToIdentity(&##varName##);

/////////////////////////////////////////////////////////////////////
// Position
/////////////////////////////////////////////////////////////////////
extern "C"
static void Transform_SetToIdentity(Transform* t)
{
	*t = {};
    t->pos = { 0, 0, 0 };
    t->scale = { 1, 1, 1 };
    M3x3_SetToIdentity(t->rotation.cells);
}

/////////////////////////////////////////////////////////////////////
// Conversion
/////////////////////////////////////////////////////////////////////
extern "C"
internal void Transform_ToM4x4(Transform* t, M4x4* result)
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

extern "C"
internal void Transform_ToM4x4NoScale(Transform* t, M4x4* result)
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

internal void Transform_FromM4x4NoScale(Transform* t, f32* m4x4)
{
    t->pos.x = m4x4[M4x4_W0];
    t->pos.y = m4x4[M4x4_W1];
    t->pos.z = m4x4[M4x4_W2];
    M3x3_CopyFromM4x4(t->rotation.cells, m4x4);
}

/////////////////////////////////////////////////////////////////////
// Scale
/////////////////////////////////////////////////////////////////////
internal void  Transform_SetScaleF(Transform* t, f32 scaleX, f32 scaleY, f32 scaleZ)
{
	t->scale.x = scaleX;
	t->scale.y = scaleY;
	t->scale.z = scaleZ;
}

// Avoid zero scales
internal void  Transform_SetScaleSafe(Transform* t, Vec3 scale)
{
    if (scale.x != 0 && scale.y != 0 && scale.z != 0)
    {
        t->scale = scale;
    }
    else
    {
        t->scale = { 1, 1, 1 };
    }
}

/////////////////////////////////////////////////////////////////////
// Rotation
/////////////////////////////////////////////////////////////////////
internal void Transform_SetRotationDegrees(Transform* t, f32 degreesX, f32 degreesY, f32 degreesZ)
{
    M3x3_SetToIdentity(t->rotation.cells);
    M3x3_RotateZ(t->rotation.cells, degreesZ * DEG2RAD);
    M3x3_RotateY(t->rotation.cells, degreesY * DEG2RAD);
	M3x3_RotateX(t->rotation.cells, degreesX * DEG2RAD);
}

internal void Transform_SetRotation(Transform* t, f32 radiansX, f32 radiansY, f32 radiansZ)
{
    M3x3_SetToIdentity(t->rotation.cells);
    M3x3_RotateZ(t->rotation.cells, radiansZ);
    M3x3_RotateY(t->rotation.cells, radiansY);
	M3x3_RotateX(t->rotation.cells, radiansX);
}

internal void Transform_ApplyChain(Transform* target, Transform** links, i32 numLinks)
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
    M4x4_CREATE(other)
    for (i32 i = 0; i < numLinks; ++i)
    {
        Transform_ToM4x4NoScale(links[i], &other);
        M4x4_Multiply(result.cells, other.cells, result.cells);
    }
    Transform_FromM4x4NoScale(target, result.cells);
}

static void Transform_Printf(Transform* t)
{
    printf("Pos %.3f, %.3f, %.3f\n", t->pos.x, t->pos.y, t->pos.z);
    printf("Rot:\n%.3f, %.3f, %.3f\n", t->rotation.x0, t->rotation.y0, t->rotation.z0);
    printf("%.3f, %.3f, %.3f\n", t->rotation.x1, t->rotation.y1, t->rotation.z1);
    printf("%.3f, %.3f, %.3f\n", t->rotation.x2, t->rotation.y2, t->rotation.z2);
}

#endif // ZE_TRANSFORM_H