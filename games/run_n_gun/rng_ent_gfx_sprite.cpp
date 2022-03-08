#include "rng_internal.h"

#define GFX_IMPACT_DURATION 0.5f

#define GFX_IMPACT_SIZE 0.4f

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void Sync(Ent2d* ent);

ze_internal EntGfxSprite* GetGfxSprite(Ent2d* ent)
{
	ZE_ASSERT(ent != NULL, "GFX sprite - ent is null")
	return &ent->d.gfxSprite;
}

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	GfxSpriteSave* save = (GfxSpriteSave*)stateHeader;
	Ent2d* ent = Sim_GetEntById(save->header.id);
	EntGfxSprite* gfx = NULL;
	if (ent != NULL)
	{
		gfx = GetGfxSprite(ent);
		gfx->data = save->data;
		Sync(ent);
	}
	else
	{
		ent = Sim_GetFreeEntity(save->header.id, ENT_TYPE_GFX_SPRITE);
		gfx = GetGfxSprite(ent);
		gfx->data = save->data;

		Vec3 pos = Vec3_FromVec2(save->data.pos, save->data.depth);
		ZRDrawObj* obj = g_engine.scenes.AddFullTextureQuad(
			g_scene,
			FALLBACK_TEXTURE_WHITE,
			{ GFX_IMPACT_SIZE, GFX_IMPACT_SIZE },
			COLOUR_F32_ORANGE);
		gfx->drawId = obj->id;

	}
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void Write(Ent2d* ent, ZEBuffer* buf)
{
	EntGfxSprite* gfx = GetGfxSprite(ent);
	ZE_BUF_INIT_PTR_IN_PLACE(save, GfxSpriteSave, buf);
	
	// header
	save->header = Ent_SaveHeaderFromEnt(ent, sizeof(GfxSpriteSave));

	// logic
	save->data = gfx->data;
}

ze_internal void Remove(Ent2d* ent)
{
	EntGfxSprite* gfx = GetGfxSprite(ent);
	g_engine.scenes.RemoveObject(g_scene, gfx->drawId);

	ent->type = ENT_TYPE_NONE;
	Sim_RemoveEntityBase(ent);
}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	EntGfxSprite* gfx = GetGfxSprite(ent);
	if (gfx->data.tick <= 0.f)
	{
		Remove(ent);
	}
	else
	{
		gfx->data.tick -= delta;
	}
}

ze_internal void Sync(Ent2d* ent)
{
	EntGfxSprite* gfx = GetGfxSprite(ent);
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, gfx->drawId);
	M3x3_SetToIdentity(obj->t.rotation.cells);
	M3x3_RotateZ(obj->t.rotation.cells, gfx->data.radians);
	obj->t.pos = Vec3_FromVec2(gfx->data.pos, gfx->data.depth);
	f32 weight = gfx->data.tick / GFX_IMPACT_DURATION;
	f32 size = GFX_IMPACT_SIZE;
	obj->t.scale = { size * weight, size * weight, 1.f };
}

ze_internal EntHitResponse Hit(Ent2d* victim, DamageHit* hit){ return {};}
ze_internal void Print(Ent2d* ent){}

ze_external void Sim_SpawnGfx(Vec2 pos, i32 subType)
{
	GfxSpriteSave save = {};
	save.header = Ent_SaveHeaderFromRaw(
		Sim_ReserveDynamicIds(1),
		ENT_EMPTY_TAG,
		ENT_TYPE_GFX_SPRITE,
		sizeof(GfxSpriteSave)
	);
	
	save.data.pos = pos;
	save.data.radians = RANDF_RANGE(0, 360) * DEG2RAD;
	save.data.tick = GFX_IMPACT_DURATION;
	// RNGPRINT("Spawn gfx at %.3f, %.3f\n", pos.x, pos.y);
	Restore(&save.header, Sim_GetRestoreTick());
}

ze_external void EntGfxSprite_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();
	type->type = ENT_TYPE_GFX_SPRITE;
	type->label = "GFX Sprite";
	type->Restore = Restore;
	type->Write = Write;
	type->Remove = Remove;
	type->Tick = Tick;
	type->Sync = Sync;
	type->Hit = Hit;
	type->Print = Print;
}
