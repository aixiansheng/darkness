#include "cbase.h"
#include "ents/pack_item.h"

struct pack_item pack_item_info[NUM_PACK_ITEMS] = {
	{ PACK_ITEM_TYPE_NULL, "null", "null" },
	{ PACK_ITEM_TYPE_HEALTH, "health", "hud/pack_items/health" }
	//{ PACK_ITEM_TYPE_XP_SHELLS, "XP shells", "hud/pack_items/xp_shells" }
};
