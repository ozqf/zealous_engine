#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick){}
ze_internal void Write(Ent2d* ent, ZEBuffer* buf){}
ze_internal void Remove(Ent2d* ent){}
ze_internal void Tick(Ent2d* ent, f32 delta){}
ze_internal void Sync(Ent2d* ent){}
ze_external EntHitResponse Hit(Ent2d* victim, DamageHit* hit){ return {};}
ze_internal void Print(Ent2d* ent){}

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
