#pragma once

#include "sim_internal.h"
#include "../ze_common/ze_random_table.h"

internal i32 Sim_CreateFlatRadialPattern(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* items,
	i32 serial,
	i32 isLocal)
{
	i32 serialIncrement = isLocal ? -1 : 1;
	f32 radians = 0;
	f32 step = FULL_ROTATION_RADIANS / (f32)def->numItems;
	for(i32 i = 0; i < def->numItems; ++i)
	{
		Vec3 dir =
		{
			cosf(radians),
			0,
			sinf(radians)
		};
		items[i].forward.x = dir.x;
		items[i].forward.y = dir.y;
		items[i].forward.z = dir.z;
		items[i].pos.x = event->pos.x + (dir.x * def->radius);
		items[i].pos.y = event->pos.y + (dir.y * def->radius);
		items[i].pos.z = event->pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;

		radians += step;
		serial += serialIncrement;
	}
	return def->numItems;
}

internal i32 Sim_CreateFlatScatterPattern(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* items,
	i32 serial,
	i32 isLocal)
{
	i32 serialIncrement = isLocal ? -1 : 1;
	f32 radians = 0;
	i32 randomIndex = event->seedIndex;
	for(i32 i = 0; i < def->numItems; ++i)
	{
		radians = ZE_Randf32(randomIndex++) * (2 * pi32);
		Vec3 dir =
		{
			cosf(radians),
			0,
			sinf(radians)
		};
		items[i].forward.x = dir.x;
		items[i].forward.y = dir.y;
		items[i].forward.z = dir.z;
		items[i].pos.x = event->pos.x + (dir.x * def->radius);
		items[i].pos.y = event->pos.y + (dir.y * def->radius);
		items[i].pos.z = event->pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;

		serial += serialIncrement;
	}
	return def->numItems;
}

internal i32 Sim_CreateFlatConePattern(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* items,
	i32 serial,
	i32 isLocal)
{
	i32 serialIncrement = isLocal ? -1 : 1;
	f32 forwardRadians = atan2f(event->forward.z, event->forward.x);
	
	if (def->numItems == 1)
	{
		// just launch one straight forward...
		items[0].forward.x = event->forward.x;
		items[0].forward.y = event->forward.y;
		items[0].forward.z = event->forward.z;
		items[0].pos.x = event->pos.x + (event->forward.x * def->radius);
		items[0].pos.y = event->pos.y + (event->forward.y * def->radius);
		items[0].pos.z = event->pos.z + (event->forward.z * def->radius);
		items[0].entSerial = serial;
		return 1;
	}

    f32 arc = def->arc;
    
	// -1 items here to cover the full sweep. Otherwise the last angle
	// is missed off
    f32 step = arc / (def->numItems - 1);
    f32 radians = forwardRadians - (arc / 2.0f);
    for (i32 i = 0; i < def->numItems; ++i)
    {
		Vec3 dir =
		{
			cosf(radians),
			0,
			sinf(radians)
		};
        items[i].forward = dir;
		items[i].pos.x = event->pos.x + (dir.x * def->radius);
		items[i].pos.y = event->pos.y + (dir.y * def->radius);
		items[i].pos.z = event->pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;

        radians += step;
		serial += serialIncrement;
    }
	return def->numItems;
}

internal i32 Sim_Create3DConePattern(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* items,
	i32 serial,
	i32 isLocal)
{
	if (def->numItems == 1)
	{
		// just launch one straight forward...
		items[0].forward.x = event->forward.x;
		items[0].forward.y = event->forward.y;
		items[0].forward.z = event->forward.z;
		items[0].pos.x = event->pos.x + (event->forward.x * def->radius);
		items[0].pos.y = event->pos.y + (event->forward.y * def->radius);
		items[0].pos.z = event->pos.z + (event->forward.z * def->radius);
		items[0].entSerial = serial;
		return 1;
	}
	// Multiple items
	i32 serialIncrement = isLocal ? -1 : 1;
	i32 randomIndex = event->seedIndex;
	#if 1
	M3x3 matrix;
	for (i32 i = 0; i < def->numItems; ++i)
    {
		M3x3_SetToIdentity(matrix.cells);
		f32 offsetX = ZE_Randf32InRange(randomIndex++, -def->arc, def->arc);
		f32 offsetY = ZE_Randf32InRange(randomIndex++, -def->arc, def->arc);
		M3x3_RotateX(matrix.cells, offsetX);
		M3x3_RotateY(matrix.cells, offsetY);
		Vec3 dir = Vec3_MultiplyByM3x3(&event->forward, matrix.cells);
        items[i].forward = dir;
		items[i].pos.x = event->pos.x + (dir.x * def->radius);
		items[i].pos.y = event->pos.y + (dir.y * def->radius);
		items[i].pos.z = event->pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;
		serial += serialIncrement;
	}
	#endif
	#if 0
    for (i32 i = 0; i < def->numItems; ++i)
    {
		// create a point some random distance away
		Vec3 origin = { 0, 0, 0 };
		Vec3 dest = { ZE_Randf32(randomIndex++), ZE_Randf32(randomIndex++), 10 };
		// Stretch out a vector and offset it.
		Vec3 offset;
		offset.x = dest.x - origin.x;
		offset.y = dest.y - origin.y;
		offset.z = dest.z - origin.z;
		// offset by a random amount
		Vec3_Normalise(&offset);
		Vec3 dir = event->forward;
		dir.x += offset.x;
		dir.y += offset.y;
		dir.z += offset.z;

        items[i].forward = dir;
		items[i].pos.x = event->pos.x + (dir.x * def->radius);
		items[i].pos.y = event->pos.y + (dir.y * def->radius);
		items[i].pos.z = event->pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;
		serial += serialIncrement;
    }
	#endif
	return def->numItems;
}

internal i32 Sim_Create3DConePattern_Old(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* items,
	i32 serial,
	i32 isLocal)
{
	if (def->numItems == 1)
	{
		// just launch one straight forward...
		items[0].forward.x = event->forward.x;
		items[0].forward.y = event->forward.y;
		items[0].forward.z = event->forward.z;
		items[0].pos.x = event->pos.x + (event->forward.x * def->radius);
		items[0].pos.y = event->pos.y + (event->forward.y * def->radius);
		items[0].pos.z = event->pos.z + (event->forward.z * def->radius);
		items[0].entSerial = serial;
		return 1;
	}
	// Multiple items
	i32 serialIncrement = isLocal ? -1 : 1;
	f32 forwardRadians = atan2f(event->forward.z, event->forward.x);
	f32 upRadians = atan2f(event->forward.y, event->forward.z);
    f32 arc = def->arc;
	// -1 items here to cover the full sweep. Otherwise the last angle
	// is missed off
    f32 step = arc / (def->numItems - 1);
    f32 forwardRadianStep = forwardRadians - (arc / 2.0f);
	f32 upRadianStep = upRadians - (arc / 2.0f);
	
    for (i32 i = 0; i < def->numItems; ++i)
    {
		Vec3 dir =
		{
			cosf(forwardRadianStep),
			sinf(upRadianStep),
			sinf(forwardRadianStep)
		};
        items[i].forward = dir;
		items[i].pos.x = event->pos.x + (dir.x * def->radius);
		items[i].pos.y = event->pos.y + (dir.y * def->radius);
		items[i].pos.z = event->pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;
        forwardRadianStep += step;
		upRadianStep += step;
		serial += serialIncrement;
    }
	return def->numItems;
}

/*
Returns number of results
Results array MUST have the capacity of items specified in the def
*/
internal i32 Sim_CreateSpawnPattern(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* results,
	i32 firstSerial,
	i32 isLocal)
{
	ZE_ASSERT(def->numItems > 0, "Pattern has zero items")
	switch (def->patternId)
	{
		case SIM_PATTERN_FLAT_RADIAL:
		return Sim_CreateFlatRadialPattern(
			event, def, results, firstSerial, isLocal);
		case SIM_PATTERN_FLAT_SCATTER:
		return Sim_CreateFlatScatterPattern(
			event, def, results, firstSerial, isLocal);
		case SIM_PATTERN_3D_CONE:
		return Sim_Create3DConePattern(
			event, def, results, firstSerial, isLocal);
		case SIM_PATTERN_FLAT_CONE:
		return Sim_CreateFlatConePattern(
			event, def, results, firstSerial, isLocal);
		
		case SIM_PATTERN_NONE:
		results[0].pos = event->pos;
		results[0].forward = event->forward;
		results[0].entSerial = event->firstSerial;
		return 1;

		default: ZE_ASSERT(0, "Unknown Pattern type") break;
	}
	return 0;
}
