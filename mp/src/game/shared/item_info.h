#ifndef __ITEM_INFO_H__
#define __ITEM_INFO_H__

struct item_info_t {
	int idx;
	int value;
	int max_health;
	int initial_health;
	int min_health;
	int team_dmg;
	float armor_factor;
	const wchar_t *display_name;
	const char *ent_name;
	const char *model;
	const char *create_sound;
	const char *killed_sound;
};

#define ARMOR_NONE		1.0f
#define ARMOR_LIGHT		0.9f
#define ARMOR_MEDIUM	0.85f
#define ARMOR_HEAVY		0.65f

#define ITEM_ARMOR_UNDEF	0
#define ITEM_ARMOR_NONE		1
#define ITEM_ARMOR_LIGHT	2
#define ITEM_ARMOR_MEDIUM	3
#define ITEM_ARMOR_HEAVY	4

typedef struct item_info_t item_info_t;

#define ITEM_TELEPORTER_IDX	0
#define ITEM_AMMO_CRATE_IDX	1
#define ITEM_MEDIPAD_IDX	2
#define ITEM_MINE_IDX		3
#define ITEM_SMG_TURRET_IDX	4
#define ITEM_DETECTOR_IDX	5
#define ITEM_MSL_TURRET_IDX	6
#define NUM_HUMAN_ITEMS		7

#define TELEPORTER_VALUE	40
#define AMMO_CRATE_VALUE	60
#define MEDIPAD_VALUE		70
#define MINE_VALUE			30
#define SMG_TURRET_VALUE	40
#define MSL_TURRET_VALUE	60
#define DETECTOR_VALUE		20

#define ITEM_EGG_IDX		0
#define ITEM_HEALER_IDX		1
#define ITEM_OBSTACLE_IDX	2
#define ITEM_SPIKER_IDX		3
#define ITEM_GASSER_IDX		4
#define NUM_SPIDER_ITEMS	5

#define EGG_VALUE			15
#define HEALER_VALUE		50
#define OBSTACLE_VALUE		20
#define SPIKER_VALUE		30
#define GASSER_VALUE		20

#define TELEPORTER_MAX_HEALTH	250
#define TELEPORTER_INIT_HEALTH	20
#define TELEPORTER_MIN_HEALTH	100
#define TELEPORTER_TEAM_DMG		0
#define TELEPORTER_ARMOR		ARMOR_HEAVY
#define TELEPORTER_DIE_SOUND	"AmmoCrate.Die"

#define DETECTOR_MAX_HEALTH		20
#define DETECTOR_INIT_HEALTH	20
#define DETECTOR_MIN_HEALTH		0
#define DETECTOR_TEAM_DMG		1
#define DETECTOR_ARMOR			ARMOR_NONE
#define DETECTOR_INIT_SOUND		"Mine.Init"

#define AMMO_CRATE_MAX_HEALTH	180
#define AMMO_CRATE_INIT_HEALTH	30
#define AMMO_CRATE_MIN_HEALTH	95
#define AMMO_CRATE_TEAM_DMG		1
#define AMMO_CRATE_ARMOR		ARMOR_LIGHT
#define AMMO_CRATE_DIE_SOUND	"AmmoCrate.Die"

#define MEDIPAD_MAX_HEALTH		150
#define MEDIPAD_INIT_HEALTH		20
#define MEDIPAD_MIN_HEALTH		75
#define MEDIPAD_TEAM_DMG		1
#define MEDIPAD_ARMOR			ARMOR_LIGHT
#define MEDIPAD_DIE_SOUND		"Medipad.Die"

#define MINE_MAX_HEALTH			10
#define MINE_INIT_HEALTH		10
#define MINE_TEAM_DMG			0
#define MINE_ARMOR				ARMOR_NONE
#define MINE_INIT_SOUND			"Mine.Init"

#define SMG_TURRET_MAX_HEALTH	140
#define SMG_TURRET_INIT_HEALTH	10
#define SMG_TURRET_MIN_HEALTH	80
#define SMG_TURRET_TEAM_DMG		1
#define SMG_TURRET_ARMOR		ARMOR_MEDIUM
#define SMG_TURRET_INIT_SOUND	"Mine.Init"
#define SMG_TURRET_DIE_SOUND	AMMO_CRATE_DIE_SOUND

#define MSL_TURRET_MAX_HEALTH	160
#define MSL_TURRET_INIT_HEALTH	10
#define MSL_TURRET_MIN_HEALTH	90
#define MSL_TURRET_TEAM_DMG		1
#define MSL_TURRET_ARMOR		ARMOR_MEDIUM
#define MSL_TURRET_INIT_SOUND	"Mine.Init"
#define MSL_TURRET_DIE_SOUND	AMMO_CRATE_DIE_SOUND

#define EGG_MAX_HEALTH			200
#define EGG_INIT_HEALTH			200
#define EGG_TEAM_DMG			0
#define EGG_ARMOR				ARMOR_HEAVY
#define EGG_DIE_SOUND			"Egg.Die"

#define HEALER_MAX_HEALTH		125
#define HEALER_INIT_HEALTH		125
#define HEALER_TEAM_DMG			1
#define HEALER_ARMOR			ARMOR_NONE
#define HEALER_DIE_SOUND		"Healer.Die"

#define OBSTACLE_MAX_HEALTH		750
#define OBSTACLE_INIT_HEALTH	200
#define OBSTACLE_TEAM_DMG		1
#define OBSTACLE_ARMOR			ARMOR_HEAVY
#define OBSTACLE_DIE_SOUND		HEALER_DIE_SOUND

#define SPIKER_MAX_HEALTH		175
#define SPIKER_INIT_HEALTH		175
#define SPIKER_TEAM_DMG			1
#define SPIKER_ARMOR			ARMOR_MEDIUM
#define SPIKER_INIT_SOUND		"Spiker.Init"
#define SPIKER_DIE_SOUND		HEALER_DIE_SOUND

#define GASSER_MAX_HEALTH		20
#define GASSER_INIT_HEALTH		20
#define GASSER_TEAM_DMG			1
#define GASSER_ARMOR			ARMOR_NONE
#define GASSER_DIE_SOUND		HEALER_DIE_SOUND

//
// ents are server side only, so put these here for now
// since they are used in client code
//
#define TELEPORTER_HULL_MIN -Vector(20,20,3)
#define TELEPORTER_HULL_MAX	Vector(20,20,3)
#define AMMO_CRATE_HULL_MIN	-Vector(30,20,20)
#define AMMO_CRATE_HULL_MAX Vector(30,20,20)
#define MEDIPAD_HULL_MIN TELEPORTER_HULL_MIN
#define MEDIPAD_HULL_MAX TELEPORTER_HULL_MAX
#define EXP_MINE_HULL_MIN -Vector(4,4,4)
#define EXP_MINE_HULL_MAX Vector(4,4,4)
#define DETECTOR_HULL_MIN EXP_MINE_HULL_MIN
#define DETECTOR_HULL_MAX EXP_MINE_HULL_MAX
#define SMG_TURRET_HULL_MIN -Vector(16,16,2)
#define SMG_TURRET_HULL_MAX Vector(16,16,25)
#define MSL_TURRET_HULL_MIN SMG_TURRET_HULL_MIN
#define MSL_TURRET_HULL_MAX SMG_TURRET_HULL_MAX
#define EGG_HULL_MIN -Vector(14,14,0)
#define EGG_HULL_MAX Vector(14,14,35)
#define HEALER_HULL_MIN -Vector(15,15,0)
#define HEALER_HULL_MAX Vector(15,15,22)
#define OBSTACLE_HULL_MIN -Vector(6.3f, 8.7f, 14.2f)
#define OBSTACLE_HULL_MAX Vector(8.6f, 6.5f, 5.5f)
#define SPIKER_HULL_MIN	-Vector(10,10,25)
#define SPIKER_HULL_MAX Vector(10,10,13)
#define GASSER_HULL_MIN -Vector(4,4,4)
#define GASSER_HULL_MAX Vector(4,4,4)

extern item_info_t human_items[NUM_HUMAN_ITEMS];
extern item_info_t spider_items[NUM_SPIDER_ITEMS];

// used by many items...
typedef struct waiting_player {
	EHANDLE player;
	float end_time;
} waiting_player_t;

#endif
