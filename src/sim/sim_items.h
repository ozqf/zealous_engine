#include "sim_internal.h"

#define SIM_INVENTORY_ITEM_COUNT 2
internal SimInventoryItem g_items[SIM_INVENTORY_ITEM_COUNT];

internal void SVI_InitItemDefs()
{
	i32 i = 0;
	g_items[i].name = "Triplestick";
	g_items[i].eventType = 0;
	g_items[i].duration = 0.5f;

	i = 1;
	g_items[i].name = "Ripper";
	g_items[i].eventType = 1;
	g_items[i].duration = 0.05f;
}
