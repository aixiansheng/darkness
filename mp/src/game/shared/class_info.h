#ifndef __CLASS_INFO_H__
#define __CLASS_INFO_H__

#include "hl2mp_gamerules.h"

struct class_info_t {
	int idx;
	int cost;
	int armor;
	int health;
	const char *pri_weapon;
	const char *sec_weapon;
	const char *grenade_type;
	const char *grenade_weapon;
	int max_grenades;
	float max_speed;
	const char *name;
	const char *model;
	float view_height;
	int fov;
	HL2MPViewVectors *vectors;
	const char *pri_ammo;
	const char *sec_ammo;
	float jump_factor;
	int snd_type_idx;
	const char *taunt_sound;
};

typedef struct class_info_t class_info_t;

#define CLASS_ENGINEER_IDX	0
#define CLASS_GRUNT_IDX		1
#define CLASS_SHOCK_IDX		2
#define CLASS_HEAVY_IDX		3
#define CLASS_COMMANDO_IDX	4
#define CLASS_EXTERMINATOR_IDX	5
#define CLASS_MECH_IDX		6

#define CLASS_BREEDER_IDX	7
#define CLASS_HATCHY_IDX	8
#define CLASS_DRONE_IDX		9
#define CLASS_KAMI_IDX		10
#define CLASS_STINGER_IDX	11
#define CLASS_GUARDIAN_IDX	12
#define CLASS_STALKER_IDX	13

//#define NUM_HUMAN_CLASSES	7
//#define NUM_SPIDER_CLASSES	7
#define NUM_CLASSES			14

#define HATCHY_DAMAGE		60.0f
#define HATCHY_HIT_RATE		0.4f

#define IS_SPIDER_CLASS(C) (\
	CLASS_BREEDER_IDX == C || \
	CLASS_HATCHY_IDX == C || \
	CLASS_DRONE_IDX == C || \
	CLASS_KAMI_IDX == C || \
	CLASS_STINGER_IDX == C || \
	CLASS_GUARDIAN_IDX == C || \
	CLASS_STALKER_IDX == C)

#define IS_HUMAN_CLASS(C) (\
	CLASS_ENGINEER_IDX == C || \
	CLASS_GRUNT_IDX == C || \
	CLASS_SHOCK_IDX == C || \
	CLASS_HEAVY_IDX == C || \
	CLASS_COMMANDO_IDX == C || \
	CLASS_EXTERMINATOR_IDX == C || \
	CLASS_MECH_IDX == C)

// WRAITH / MEDIC?

#define NORMAL_SPEED		320.0f
// unit speeds
#define ENGINEER_SPEED		(NORMAL_SPEED)
#define GRUNT_SPEED			(NORMAL_SPEED)
#define SHOCK_SPEED			(NORMAL_SPEED)
#define HEAVY_SPEED			(NORMAL_SPEED * 0.8f)
#define COMMANDO_SPEED		(NORMAL_SPEED * 1.15f)
#define EXTERM_SPEED		(NORMAL_SPEED * 0.9f)
#define MECH_SPEED			(NORMAL_SPEED * 0.8f)

#define BREEDER_SPEED		(NORMAL_SPEED)
#define HATCHY_SPEED		(NORMAL_SPEED * 1.45f)
#define DRONE_SPEED			(NORMAL_SPEED * 1.25f)
#define KAMI_SPEED			HATCHY_SPEED
#define STINGER_SPEED		(NORMAL_SPEED * 1.3f)
#define GUARDIAN_SPEED		(NORMAL_SPEED)
#define STALKER_SPEED		(NORMAL_SPEED * 1.25f)

// models
#define ENGINEER_MODEL		"models/engy.mdl"
#define GRUNT_MODEL			"models/humans/group03/female_01.mdl"
#define SHOCK_MODEL			"models/humans/group03/male_01.mdl"
#define HEAVY_MODEL			"models/Combine_Soldier.mdl"
#define COMMANDO_MODEL		"models/Combine_Soldier_Prisonguard.mdl"
#define EXTERM_MODEL		"models/Combine_Super_Soldier.mdl"
#define MECH_MODEL			"models/mech.mdl" 

#define BREEDER_MODEL		"models/breeder.mdl"
#define HATCHY_MODEL		"models/hatchy.mdl"
#define DRONE_MODEL			"models/drone.mdl"
#define KAMI_MODEL			"models/hatchy.mdl"
#define STINGER_MODEL		"models/stinger.mdl"
#define GUARDIAN_MODEL		"models/guardian.mdl"
#define STALKER_MODEL		"models/dkstalker.mdl"

// foot sounds
// - these are indexes into the sound tables in hl2mp_player_shared.cpp
//
#define ENGY_SND_TYPE		8
#define GRUNT_SND_TYPE		0
#define SHOCK_SND_TYPE		GRUNT_SND_TYPE
#define HEAVY_SND_TYPE		2
#define COMMANDO_SND_TYPE	1
#define EXTERM_SND_TYPE		0
#define MECH_SND_TYPE		9

#define BREEDER_SND_TYPE	3
#define HATCHY_SND_TYPE		4
#define DRONE_SND_TYPE		5
#define KAMI_SND_TYPE		4
#define STINGER_SND_TYPE	6
#define GUARDIAN_SND_TYPE	3
#define STALKER_SND_TYPE	7

// heights
#define STANDARD_HEIGHT		1
#define BREEDER_HEIGHT		0.3
#define HATCHY_HEIGHT		0.2
#define DRONE_HEIGHT		STANDARD_HEIGHT
#define KAMI_HEIGHT			HATCHY_HEIGHT
#define STINGER_HEIGHT		0.3
#define GUARDIAN_HEIGHT		0.3
#define STALKER_HEIGHT		STANDARD_HEIGHT

#define BREEDER_JUMP_FACTOR		1.3f
#define HATCHY_JUMP_FACTOR		1.9f
#define DRONE_JUMP_FACTOR		3.5f
#define KAMI_JUMP_FACTOR		HATCHY_JUMP_FACTOR
#define STINGER_JUMP_FACTOR		1.55f
#define GUARDIAN_JUMP_FACTOR	1.3f
#define STALKER_JUMP_FACTOR		1.0f

#define ENGY_JUMP_FACTOR		1.2f
#define GRUNT_JUMP_FACTOR		1.2f
#define SHOCK_JUMP_FACTOR		1.2f
#define HEAVY_JUMP_FACTOR		0.9f
#define COMMANDO_JUMP_FACTOR	1.85f
#define EXT_JUMP_FACTOR			1.2f
#define MECH_JUMP_FACTOR		0.4f

#define ENGINEER_NAME		"Engineer"
#define GRUNT_NAME			"Grunt"
#define SHOCK_NAME			"Shock Trooper"
#define HEAVY_NAME			"Heavy Trooper"
#define COMMANDO_NAME		"Commando"
#define EXTERM_NAME			"Exterminator"
#define MECH_NAME			"Mech"

#define BREEDER_NAME		"Breeder"
#define HATCHY_NAME			"Hatchling"
#define DRONE_NAME			"Drone"
#define KAMI_NAME			"Kamikazi"
#define STINGER_NAME		"Stinger"
#define GUARDIAN_NAME		"Guardian"
#define STALKER_NAME		"Stalker"

#define WEAPON_NULL			"none"
#define WEAPON_WELDER		"weapon_engy"
#define WEAPON_DESTROYER	"weapon_engy_destroy"
#define WEAPON_SMG			"weapon_smg1"
#define WEAPON_PISTOL		"weapon_pistol"
#define WEAPON_SHOTGUN		"weapon_shotgun"
#define WEAPON_RPG			"weapon_rpg"
#define WEAPON_SILENCED_SMG	"weapon_silenced_smg1"
#define WEAPON_357			"weapon_357"
#define WEAPON_PLASMA		"weapon_plasma_rifle"
#define WEAPON_CANON		"weapon_plasma_canon"

#define GRENADE_NULL		"none"
#define GRENADE_FRAG		"grenade"
#define GRENADE_ACID		"grenade_acid"
#define GRENADE_GUARD		"grenade_guardian"
#define GRENADE_SPIKE		"grenade_guardian"
#define GRENADE_SMOKE		"grenade_smk"

#define WEAPON_SMOKE_GREN	"weapon_smk_gren"
#define WEAPON_GUARD_GREN	"weapon_guard_gren"
#define WEAPON_SPIKE_GREN	"weapon_stalk_gren"
#define WEAPON_ACID_GREN	"weapon_acid_gren"
#define WEAPON_FRAG_GREN	"weapon_frag"

// #define GRENADE_FLARE		"grenade_flare"

#define WEAPON_BREEDER			"weapon_breeder"
#define WEAPON_HATCHY			"weapon_hatchy"
#define WEAPON_DRONE_SLASH		"weapon_drone_slash"
#define WEAPON_DRONE_SPIT		"weapon_drone_spit"
#define WEAPON_KAMI				"weapon_hatchy"
#define WEAPON_STINGER_FIRE		"weapon_stinger_fire"
#define WEAPON_GUARDIAN			"weapon_guardian_slash"
#define WEAPON_STALKER_SLASH	"weapon_stalker_slash"
#define WEAPON_STALKER_SPIKE	"weapon_stalker_spike"

// Field of view (vertical)
#define BREEDER_FOV			65
#define HATCHY_FOV			120
#define DRONE_FOV			85
#define KAMI_FOV			HATCHY_FOV
#define STINGER_FOV			55
#define GUARDIAN_FOV		75
#define STALKER_FOV			75

#define HUMAN_FOV			75

// ammo types
#define AMMO_NULL		"none"
#define AMMO_SMG		"SMG1"
#define AMMO_SSMG		"SSMG"
#define AMMO_PISTOL		"Pistol"
#define AMMO_SHOTGUN	"Buckshot"
#define AMMO_RPG		"RPG_Round"
#define AMMO_357		"357"
#define AMMO_SPIKE		"spike"
#define AMMO_PLASMA		"plasma"
#define AMMO_PLASMA_CANON "plasma_bolt"

#define ENGY_TAUNT		"Engy.Taunt"
#define GRUNT_TAUNT		"Grunt.Taunt"
#define SHOCK_TAUNT		"Shock.Taunt"
#define HEAVY_TAUNT		"Heavy.Taunt"
#define COMMANDO_TAUNT	"Commando.Taunt"
#define EXTERM_TAUNT	"Exterm.Taunt"
#define MECH_TAUNT		"Mech.Taunt"

#define BREEDER_TAUNT	"Breeder.Taunt"
#define HATCHY_TAUNT	"Hatchy.Taunt"
#define DRONE_TAUNT		"Drone.Taunt"
#define KAMI_TAUNT		HATCHY_TAUNT
#define STINGER_TAUNT	"Stinger.Taunt"
#define GUARDIAN_TAUNT	"Guardian.Taunt"
#define STALKER_TAUNT	"Stalker.Taunt"

extern class_info_t dk_classes[NUM_CLASSES];

#endif
