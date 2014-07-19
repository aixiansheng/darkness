#include "cbase.h"
#include "class_info.h"

static HL2MPViewVectors human_vectors(
	Vector( 0, 0, 64 ),       //VEC_VIEW (m_vView) 

	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 28 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  60 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static HL2MPViewVectors breeder_vectors(
	Vector( 0, 0, 32 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  45 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  45 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 32 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  30 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static HL2MPViewVectors hatchy_vectors(
	Vector( 0, 0, 13 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-15, -15, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 15,  15,  15 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-15, -15, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 15,  15,  15 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 13 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-15, -15, -15 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 15,  15,  15 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 13 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-15, -15, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 15,  15,  15 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static HL2MPViewVectors drone_vectors(
	Vector( 0, 0, 45 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  55 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  55 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 45 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  55 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static HL2MPViewVectors stinger_vectors(
	Vector( 0, 0, 35 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-18, -18, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 18,  18,  54 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-18, -18, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 18,  18,  54 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 35 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-18, -18, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 18,  18,  54 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static HL2MPViewVectors guardian_vectors(
	Vector( 0, 0, 40 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-18, -18, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 18,  18,  54 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-18, -18, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 18,  18,  54 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 40 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-18, -18, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 18,  18,  48 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

// for mech/stalker
static HL2MPViewVectors large_vectors(
	Vector( 0, 0, 85 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-25, -25, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 25,  25,  85 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-25, -25, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 25,  25,  85 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 85 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-25, -25, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 25,  25,  80 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

class_info_t dk_classes[NUM_CLASSES] = {
	{	
		CLASS_ENGINEER_IDX, 
		0, 
		0, 
		75, 
		WEAPON_WELDER, 
		WEAPON_DESTROYER, 
		GRENADE_NULL,
		WEAPON_NULL, 
		0,
		ENGINEER_SPEED, 
		ENGINEER_NAME, 
		ENGINEER_MODEL, 
		STANDARD_HEIGHT, 
		HUMAN_FOV,
		&human_vectors, 
		AMMO_NULL,
		AMMO_NULL, 
		ENGY_JUMP_FACTOR,
		ENGY_SND_TYPE,
		ENGY_TAUNT
	},
	{	
		CLASS_GRUNT_IDX, 
		0, 
		25, 
		100,
		WEAPON_SMG, 
		WEAPON_PISTOL,
		GRENADE_NULL,
		WEAPON_NULL,
		0, 
		GRUNT_SPEED, 
		GRUNT_NAME, 
		GRUNT_MODEL, 
		STANDARD_HEIGHT,
		HUMAN_FOV,
		&human_vectors,
		AMMO_SMG,
		AMMO_PISTOL, 
		GRUNT_JUMP_FACTOR,
		GRUNT_SND_TYPE,
		GRUNT_TAUNT
	},
	{
		CLASS_SHOCK_IDX, 
		1, 
		25,
		100,
		WEAPON_SHOTGUN,
		WEAPON_PISTOL,
		GRENADE_SMOKE,
		WEAPON_SMOKE_GREN,
		2,
		SHOCK_SPEED, 
		SHOCK_NAME,
		SHOCK_MODEL, 
		STANDARD_HEIGHT,
		HUMAN_FOV,
		&human_vectors,
		AMMO_SHOTGUN, 
		AMMO_PISTOL,
		SHOCK_JUMP_FACTOR,
		SHOCK_SND_TYPE,
		SHOCK_TAUNT
	},
	{ 
		CLASS_HEAVY_IDX,
		2, 
		100,
		180,
		WEAPON_RPG,
		WEAPON_PISTOL, 
		GRENADE_NULL,
		WEAPON_NULL,
		0, 
		HEAVY_SPEED,
		HEAVY_NAME, 
		HEAVY_MODEL,
		STANDARD_HEIGHT,
		HUMAN_FOV, 
		&human_vectors, 
		AMMO_RPG, 
		AMMO_PISTOL, 
		HEAVY_JUMP_FACTOR,
		HEAVY_SND_TYPE,
		HEAVY_TAUNT,
	},	
	{
		CLASS_COMMANDO_IDX,
		3, 
		50,
		200,
		WEAPON_SILENCED_SMG,
		WEAPON_357,
		GRENADE_FRAG, 
		WEAPON_FRAG_GREN,
		2, 
		COMMANDO_SPEED, 
		COMMANDO_NAME,
		COMMANDO_MODEL,
		STANDARD_HEIGHT, 
		HUMAN_FOV, 
		&human_vectors, 
		AMMO_SSMG, 
		AMMO_357, 
		COMMANDO_JUMP_FACTOR,
		COMMANDO_SND_TYPE,
		COMMANDO_TAUNT
	},
	{ 
		CLASS_EXTERMINATOR_IDX, 
		4, 
		0, 
		125,
		WEAPON_PLASMA,
		WEAPON_NULL,
		GRENADE_FRAG,
		WEAPON_FRAG_GREN,
		4, 
		EXTERM_SPEED, 
		EXTERM_NAME, 
		EXTERM_MODEL,
		STANDARD_HEIGHT,
		HUMAN_FOV, 
		&human_vectors, 
		AMMO_PLASMA,
		AMMO_NULL, 
		EXT_JUMP_FACTOR,
		EXTERM_SND_TYPE,
		EXTERM_TAUNT
	},
	{ 
		CLASS_MECH_IDX,
		5, 
		550,
		120,
		WEAPON_CANON,
		WEAPON_NULL, 
		GRENADE_NULL,
		WEAPON_NULL, 
		0, 
		MECH_SPEED, 
		MECH_NAME, 
		MECH_MODEL,
		STANDARD_HEIGHT,
		HUMAN_FOV,
		&large_vectors, 
		AMMO_PLASMA_CANON, 
		AMMO_NULL, 
		MECH_JUMP_FACTOR,
		MECH_SND_TYPE,
		MECH_TAUNT
	},
	
	//
	// SPIDER CLASSES
	//

	{ 
		CLASS_BREEDER_IDX, 
		0, 
		0, 
		150,
		WEAPON_BREEDER,
		WEAPON_NULL, 
		GRENADE_NULL,
		WEAPON_NULL, 
		0, 
		BREEDER_SPEED, 
		BREEDER_NAME, 
		BREEDER_MODEL,
		BREEDER_HEIGHT,
		BREEDER_FOV,
		&breeder_vectors, 
		AMMO_NULL,
		AMMO_NULL,
		BREEDER_JUMP_FACTOR,
		BREEDER_SND_TYPE,
		BREEDER_TAUNT
	},
	{ 
		CLASS_HATCHY_IDX,
		0, 
		0,
		20,
		WEAPON_HATCHY,
		WEAPON_NULL, 
		GRENADE_NULL,
		WEAPON_NULL,
		0, 
		HATCHY_SPEED,
		HATCHY_NAME, 
		HATCHY_MODEL, 
		HATCHY_HEIGHT,
		HATCHY_FOV,
		&hatchy_vectors,
		AMMO_NULL, 
		AMMO_NULL,
		HATCHY_JUMP_FACTOR,
		HATCHY_SND_TYPE,
		HATCHY_TAUNT
	},
	{
		CLASS_DRONE_IDX,
		1,
		25,
		100,
		WEAPON_DRONE_SLASH,
		WEAPON_NULL,
		GRENADE_NULL,
		WEAPON_NULL, 
		0, 
		DRONE_SPEED, 
		DRONE_NAME,
		DRONE_MODEL, 
		DRONE_HEIGHT,
		DRONE_FOV, 
		&drone_vectors,
		AMMO_NULL,
		AMMO_NULL, 
		DRONE_JUMP_FACTOR,
		DRONE_SND_TYPE,
		DRONE_TAUNT
	},
	{ 
		CLASS_KAMI_IDX,
		2, 
		10,
		20, 
		WEAPON_KAMI, 
		WEAPON_NULL,
		GRENADE_NULL,
		WEAPON_NULL,
		0,
		KAMI_SPEED, 
		KAMI_NAME, 
		KAMI_MODEL,
		KAMI_HEIGHT,
		KAMI_FOV, 
		&hatchy_vectors,
		AMMO_NULL,
		AMMO_NULL, 
		KAMI_JUMP_FACTOR,
		KAMI_SND_TYPE,
		KAMI_TAUNT
	},
	{ 
		CLASS_STINGER_IDX, 
		3, 
		0,
		200,
		WEAPON_STINGER_FIRE,
		WEAPON_NULL,
		GRENADE_ACID,
		WEAPON_ACID_GREN,
		3, 
		STINGER_SPEED, 
		STINGER_NAME,
		STINGER_MODEL,
		STINGER_HEIGHT,
		STINGER_FOV,
		&stinger_vectors, 
		AMMO_NULL, 
		AMMO_NULL, 
		STINGER_JUMP_FACTOR,
		STINGER_SND_TYPE,
		STINGER_TAUNT
	},
	{ 
		CLASS_GUARDIAN_IDX,
		4, 
		30,
		200,
		WEAPON_GUARDIAN,
		WEAPON_NULL, 
		GRENADE_GUARD,
		WEAPON_GUARD_GREN,
		3,
		GUARDIAN_SPEED,
		GUARDIAN_NAME,
		GUARDIAN_MODEL,
		GUARDIAN_HEIGHT, 
		GUARDIAN_FOV,
		&guardian_vectors,
		AMMO_NULL,
		AMMO_NULL,
		GUARDIAN_JUMP_FACTOR,
		GUARDIAN_SND_TYPE,
		GUARDIAN_TAUNT
	},
	{ 
		CLASS_STALKER_IDX,
		5, 
		250,
		200,
		WEAPON_STALKER_SLASH, 
		WEAPON_NULL, 
		GRENADE_SPIKE,
		WEAPON_SPIKE_GREN, 
		3, 
		STALKER_SPEED, 
		STALKER_NAME, 
		STALKER_MODEL, 
		STALKER_HEIGHT,
		STALKER_FOV,
		&large_vectors, 
		AMMO_SPIKE, 
		AMMO_NULL, 
		STALKER_JUMP_FACTOR,
		STALKER_SND_TYPE,
		STALKER_TAUNT
	}
};
