#include "sim_internal.h"

#define SIM_INVENTORY_ITEM_COUNT 2
internal SimInventoryItem g_items[SIM_INVENTORY_ITEM_COUNT];

internal void SVI_InitItemDefs()
{
	i32 i = 0;
	g_items[i].name = "Triplestick";
	g_items[i].eventType = SIM_ITEM_EVENT_TYPE_PROJECTILE;
	g_items[i].factoryType = SIM_FACTORY_TYPE_PROJ_PLAYER;
	g_items[i].eventCount = SIM_PLAYER_SHOTGUN_PELLETS;
	g_items[i].duration = 0.5f;

	i = 1;
	g_items[i].name = "Ripper";
	g_items[i].eventType = SIM_ITEM_EVENT_TYPE_PROJECTILE;
	g_items[i].factoryType = SIM_FACTORY_TYPE_PROJ_PLAYER;
	g_items[i].eventCount = 1;
	g_items[i].duration = 0.05f;
}
