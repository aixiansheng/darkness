#include "cbase.h"
#include "item_info.h"

item_info_t dk_items[NUM_ITEMS] = {
	{ 
		ITEM_TELEPORTER_IDX,
		TELEPORTER_VALUE,
		TELEPORTER_MAX_HEALTH,
		TELEPORTER_INIT_HEALTH,
		TELEPORTER_MIN_HEALTH,
		TELEPORTER_TEAM_DMG,
		TELEPORTER_ARMOR,
		"Teleporter",
		"ent_teleporter",
		"models/teleporter.mdl",
		NULL,
		TELEPORTER_DIE_SOUND
	},
	{
		ITEM_AMMO_CRATE_IDX,
		AMMO_CRATE_VALUE,
		AMMO_CRATE_MAX_HEALTH,
		AMMO_CRATE_INIT_HEALTH,
		AMMO_CRATE_MIN_HEALTH,
		AMMO_CRATE_TEAM_DMG,
		AMMO_CRATE_ARMOR,
		"Ammo Crate",
		"ent_ammo_crate",
		"models/Items/ammocrate_smg1.mdl",
		NULL,
		AMMO_CRATE_DIE_SOUND
	},
	{
		ITEM_MEDIPAD_IDX,
		MEDIPAD_VALUE,
		MEDIPAD_MAX_HEALTH,
		MEDIPAD_INIT_HEALTH,
		MEDIPAD_MIN_HEALTH,
		MEDIPAD_TEAM_DMG,
		MEDIPAD_ARMOR,
		"Medi-pad",
		"ent_medipad",
		"models/teleporter.mdl",
		NULL,
		MEDIPAD_DIE_SOUND
	},
	{
		ITEM_MINE_IDX,
		MINE_VALUE,
		MINE_MAX_HEALTH,
		MINE_INIT_HEALTH,
		0,
		MINE_TEAM_DMG,
		MINE_ARMOR,
		"Mine",
		"ent_mine",
		"models/mine.mdl",
		MINE_INIT_SOUND,
		NULL
	},
	{
		ITEM_SMG_TURRET_IDX,
		SMG_TURRET_VALUE,
		SMG_TURRET_MAX_HEALTH,
		SMG_TURRET_INIT_HEALTH,
		SMG_TURRET_MIN_HEALTH,
		SMG_TURRET_TEAM_DMG,
		SMG_TURRET_ARMOR,
		"MG Turret",
		"ent_mg_turret",
		"models/turret_base.mdl",
		SMG_TURRET_INIT_SOUND,
		SMG_TURRET_DIE_SOUND
	},
	{
		ITEM_DETECTOR_IDX,
		DETECTOR_VALUE,
		DETECTOR_MAX_HEALTH,
		DETECTOR_INIT_HEALTH,
		0,
		DETECTOR_TEAM_DMG,
		DETECTOR_ARMOR,
		"Detector",
		"ent_detector",
		"models/mine.mdl",
		DETECTOR_INIT_SOUND,
		NULL
	},
	{
		ITEM_MSL_TURRET_IDX,
		MSL_TURRET_VALUE,
		MSL_TURRET_MAX_HEALTH,
		MSL_TURRET_INIT_HEALTH,
		MSL_TURRET_MIN_HEALTH,
		MSL_TURRET_TEAM_DMG,
		MSL_TURRET_ARMOR,
		"Missile Turret",
		"ent_msl_turret",
		"models/turret_base.mdl",
		MSL_TURRET_INIT_SOUND,
		MSL_TURRET_DIE_SOUND
	},
	{
		ITEM_EGG_IDX,
		EGG_VALUE,
		EGG_MAX_HEALTH,
		EGG_INIT_HEALTH,
		0,
		EGG_TEAM_DMG,
		EGG_ARMOR,
		"Egg",
		"ent_egg",
		"models/egg.mdl",
		NULL,
		EGG_DIE_SOUND
	},
	{
		ITEM_HEALER_IDX,
		HEALER_VALUE,
		HEALER_MAX_HEALTH,
		HEALER_INIT_HEALTH,
		0,
		HEALER_TEAM_DMG,
		HEALER_ARMOR,
		"Healer",
		"ent_healer",
		"models/healer.mdl",
		NULL,
		HEALER_DIE_SOUND
	},
	{
		ITEM_OBSTACLE_IDX,
		OBSTACLE_VALUE,
		OBSTACLE_MAX_HEALTH,
		OBSTACLE_INIT_HEALTH,
		0,
		OBSTACLE_TEAM_DMG,
		OBSTACLE_ARMOR,
		"Obstacle",
		"ent_obstacle",
		"models/obstacle.mdl",
		NULL,
		OBSTACLE_DIE_SOUND
	},
	{
		ITEM_SPIKER_IDX,
		SPIKER_VALUE,
		SPIKER_MAX_HEALTH,
		SPIKER_INIT_HEALTH,
		0,
		SPIKER_TEAM_DMG,
		SPIKER_ARMOR,
		"Spiker",
		"ent_spiker",
		"models/spiker.mdl",
		SPIKER_INIT_SOUND,
		SPIKER_DIE_SOUND
	},
	{
		ITEM_GASSER_IDX,
		GASSER_VALUE,
		GASSER_MAX_HEALTH,
		GASSER_INIT_HEALTH,
		0,
		GASSER_TEAM_DMG,
		GASSER_ARMOR,
		"Gasser",
		"ent_gasser",
		"models/Gibs/HGIBS.mdl",
		NULL,
		GASSER_DIE_SOUND
	}
};
