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
		items[i].pos.x = event->xForm.pos.x + (dir.x * def->radius);
		items[i].pos.y = event->xForm.pos.y + (dir.y * def->radius);
		items[i].pos.z = event->xForm.pos.z + (dir.z * def->radius);
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
		items[i].pos.x = event->xForm.pos.x + (dir.x * def->radius);
		items[i].pos.y = event->xForm.pos.y + (dir.y * def->radius);
		items[i].pos.z = event->xForm.pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;

		serial += serialIncrement;
	}
	return def->numItems;
}

internal i32 Sim_Create3DScatterPattern(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* items,
	i32 serial,
	i32 isLocal)
{
	i32 serialIncrement = isLocal ? -1 : 1;
	i32 randomIndex = event->seedIndex;
	for(i32 i = 0; i < def->numItems; ++i)
	{
		Vec3 offset;
		offset.x = ZE_Randf32InRange(randomIndex++, -def->radius, def->radius);
		offset.y = ZE_Randf32InRange(randomIndex++, -def->radius, def->radius);
		offset.z = ZE_Randf32InRange(randomIndex++, -def->radius, def->radius);
		Vec3 dir = offset;
		Vec3_Normalise(&dir);
		items[i].forward.x = dir.x;
		items[i].forward.y = dir.y;
		items[i].forward.z = dir.z;
		items[i].pos.x = event->xForm.pos.x + offset.x;
		items[i].pos.y = event->xForm.pos.y + offset.y;
		items[i].pos.z = event->xForm.pos.z + offset.z;
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
	f32 forwardRadians = atan2f(event->xForm.rotation.zAxis.z, event->xForm.rotation.zAxis.x);
	
	if (def->numItems == 1)
	{
		// just launch one straight forward...
		items[0].forward.x = event->xForm.rotation.zAxis.x;
		items[0].forward.y = event->xForm.rotation.zAxis.y;
		items[0].forward.z = event->xForm.rotation.zAxis.z;
		items[0].pos.x = event->xForm.pos.x + (event->xForm.rotation.zAxis.x * def->radius);
		items[0].pos.y = event->xForm.pos.y + (event->xForm.rotation.zAxis.y * def->radius);
		items[0].pos.z = event->xForm.pos.z + (event->xForm.rotation.zAxis.z * def->radius);
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
		items[i].pos.x = event->xForm.pos.x + (dir.x * def->radius);
		items[i].pos.y = event->xForm.pos.y + (dir.y * def->radius);
		items[i].pos.z = event->xForm.pos.z + (dir.z * def->radius);
		items[i].entSerial = serial;

        radians += step;
		serial += serialIncrement;
    }
	return def->numItems;
}

// Quake 2 style
internal i32 Sim_Create3DConePattern(
	SimSpawnBase* event,
	SimSpawnPatternDef* def,
	SimSpawnPatternItem* items,
	i32 serial,
	i32 isLocal)
{
	// Multiple items
	i32 serialIncrement = isLocal ? -1 : 1;
	i32 randomIndex = event->seedIndex;
	
	Vec3 pos = event->xForm.pos;
	Vec3 forward = event->xForm.rotation.zAxis;
	Vec3 up = event->xForm.rotation.yAxis;
	Vec3 right = event->xForm.rotation.xAxis;

	// always launch one straight forward
	items[0].forward = forward;
	items[0].pos.x = pos.x + (event->xForm.rotation.zAxis.x * def->radius);
	items[0].pos.y = pos.y + (event->xForm.rotation.zAxis.y * def->radius);
	items[0].pos.z = pos.z + (event->xForm.rotation.zAxis.z * def->radius);
	items[0].entSerial = serial;
	serial += serialIncrement;

	f32 inflatedArc = def->arc * 10000;
	// now the rest, starting from second
	for (i32 i = 1; i < def->numItems; ++i)
	{
		SimSpawnPatternItem* item = &items[i];
		f32 offsetX = ZE_Randf32InRange(randomIndex++, -inflatedArc, inflatedArc);
		f32 offsetY = ZE_Randf32InRange(randomIndex++, -inflatedArc, inflatedArc);

		// create an offset forward vector:
		Vec3 end = Vec3_VectorMA(event->xForm.pos, 8192, forward);
		end = Vec3_VectorMA(end, offsetX, right); // deflect horizontal
		end = Vec3_VectorMA(end, offsetY, up); // deflect vertical
		// printf("\tPos: %.3f, %.3f, %.3f to end %.3f, %.3f, %.3f\n",
		// 	pos.x, pos.y, pos.z, end.x, end.y, end.z);
		Vec3 r;
		r.x = end.x - pos.x;
		r.y = end.y - pos.y;
		r.z = end.z - pos.z;
		Vec3_Normalise(&r);

		item->forward = r;
		item->pos.x = pos.x + (r.x * def->radius);
		item->pos.y = pos.y + (r.y * def->radius);
		item->pos.z = pos.z + (r.z * def->radius);
		item->entSerial = serial;

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
		case SIM_PATTERN_3D_SCATTER:
		return Sim_Create3DScatterPattern(
			event, def, results, firstSerial, isLocal);
		case SIM_PATTERN_FLAT_CONE:
		return Sim_CreateFlatConePattern(
			event, def, results, firstSerial, isLocal);
		
		case SIM_PATTERN_NONE:
		results[0].pos = event->xForm.pos;
		results[0].forward = event->xForm.rotation.zAxis;
		results[0].entSerial = event->firstSerial;
		return 1;

		default: ZE_ASSERT(0, "Unknown Pattern type") break;
	}
	return 0;
}
