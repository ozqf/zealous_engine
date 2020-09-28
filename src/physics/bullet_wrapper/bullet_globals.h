#pragma once

#include "bullet_internal_types.h"

// #define PHYS_MAX_WORLDS 8
// internal ZBulletWorld g_worlds[PHYS_MAX_WORLDS];
// internal i32 g_nextWorldIndex = 0;

internal PhysErrorHandler g_errorHandler = NULL;
//internal ZBulletWorld g_world;
//internal ZEBuffer g_input;
//internal ZEBuffer g_output;
//internal PhysicsTestState g_physTest;

internal Vec3 g_testPos;

#define PHYS_MAX_BODIES 2048
#define PHYS_MAX_OVERLAPS 2048

//internal PhysBodyHandle g_bodies[PHYS_MAX_BODIES];
//internal PhysOverlapPair g_overlapPairs[PHYS_MAX_OVERLAPS];

internal const i32 g_debugStringSize = 1024;
internal char g_debugString[g_debugStringSize];
