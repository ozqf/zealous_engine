#ifndef RNG_ANIMATIONS
#define RNG_ANIMATIONS

#include "rng_internal.h"

internal RNGQuadUVs g_animUVs[ANIM_INDEX_LAST];
internal i32 g_bAnimsInit = NO; 

ze_external void RNG_AnimationsInit()
{
	g_animUVs[ANIM_INDEX_WORLD].min = { 0.0, 0.75 };
	g_animUVs[ANIM_INDEX_WORLD].max = { 0.25, 1.0 };
	
	g_animUVs[ANIM_INDEX_PLAYER_1].min = { 0.0, 0.75 };
	g_animUVs[ANIM_INDEX_PLAYER_1].max = { 0.25, 1.0 };
	
	g_animUVs[ANIM_INDEX_PROJECTILE_1].min = { 0.5, 0.75 };
	g_animUVs[ANIM_INDEX_PROJECTILE_1].max = { 0.75, 1.0 };
	
	g_animUVs[ANIM_INDEX_ENEMY_GRUNT_1].min = { 0.25, 0.75 };
	g_animUVs[ANIM_INDEX_ENEMY_GRUNT_1].max = { 0.5, 1.0 };
	
	g_animUVs[ANIM_INDEX_ENEMY_DEBRIS_1].min = { 0.0, 0.75 };
	g_animUVs[ANIM_INDEX_ENEMY_DEBRIS_1].max = { 0.25, 1.0 };
	
	g_bAnimsInit = YES;
}

ze_external RNGQuadUVs RNG_GetQuadUVs(i32 index)
{
	if (!g_bAnimsInit) { RNG_AnimationsInit(); }
	if (index < 0) { index = 0; }
	if (index >= ANIM_INDEX_LAST) { index = 0; }
	return g_animUVs[index];
}

ze_external void RNG_SetQuadUVs(i32 index, Vec2* min, Vec2* max)
{
	RNGQuadUVs quad = RNG_GetQuadUVs(index);
	*min = quad.min;
	*max = quad.max;
}

#endif // RNG_ANIMATIONS
