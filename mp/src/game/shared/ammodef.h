//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Holds defintion for game ammo types
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef AI_AMMODEF_H
#define AI_AMMODEF_H

#ifdef _WIN32
#pragma once
#endif

#include "class_info.h"

// human ammo types
#define WPN_DEFAULT			0
#define WPN_ENGY			1
#define WPN_PISTOL			2
#define WPN_SMG				3
#define WPN_SHOTGUN_BUCK	4
#define WPN_SHOTGUN_XP		5
#define WPN_RPG				6
#define WPN_SILENCED_SMG	7
#define WPN_357				8
#define WPN_C4				9
#define WPN_FRAG			10
#define WPN_PLASMA_RIFLE	11
#define WPN_RAILGUN			12
#define WPN_PLASMA_CANON	13

// spider ammo types
#define WPN_HATCHY_SLASH	14
#define WPN_DRONE_SLASH		15
#define WPN_DRONE_SPIT		16
#define WPN_KAMI_SLASH		17
#define WPN_STINGER_SLASH	18
#define WPN_STINGER_FIRE	19
#define WPN_ACID_GREN		20
#define WPN_GUARDIAN_SLASH	21
#define WPN_GUARDIAN_SPIKE	22
#define WPN_STALKER_SLASH	23
#define WPN_STALKER_GREN	24
#define WPN_STALKER_SPIKE	25
#define WPN_KAMI_XP			26

// item ammo types
#define WPN_OBSTACLE		27
#define WPN_SPIKER			28
#define WPN_GASSER			29
#define WPN_XP_MINE			30
#define WPN_MG_TURRET		31
#define WPN_MSL_TURRET		32
#define WPN_INFESTED		33

#define WPN_MAX				34

#define WPN_FACTOR(ARRAY, WPN_IDX, CLASS_IDX, FACTOR) ARRAY[WPN_IDX][CLASS_IDX] = FACTOR;

class ConVar;

struct Ammo_t 
{
	char 		*pName;
	int			nDamageType;
	int			eTracerType;
	float		physicsForceImpulse;
	int			nMinSplashSize;
	int			nMaxSplashSize;

	int			nFlags;
	int			weapon_number;

	// Values for player/NPC damage and carrying capability
	// If the integers are set, they override the CVars
	int			pPlrDmg;		// CVar for player damage amount
	int			pNPCDmg;		// CVar for NPC damage amount
	int			pMaxCarry;		// CVar for maximum number can carry
	const ConVar*		pPlrDmgCVar;	// CVar for player damage amount
	const ConVar*		pNPCDmgCVar;	// CVar for NPC damage amount
	const ConVar*		pMaxCarryCVar;	// CVar for maximum number can carry
	
};

// Used to tell AmmoDef to use the cvars, not the integers
#define		USE_CVAR		-1
// Ammo is infinite
#define		INFINITE_AMMO	-2

enum AmmoTracer_t
{
	TRACER_NONE,
	TRACER_LINE,
	TRACER_RAIL,
	TRACER_BEAM,
	TRACER_LINE_AND_WHIZ,
};

enum AmmoFlags_t
{
	AMMO_FORCE_DROP_IF_CARRIED = 0x1,
	AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER = 0x2,
};


#include "shareddefs.h"

//=============================================================================
//	>> CAmmoDef
//=============================================================================
class CAmmoDef
{

public:
	int		m_nAmmoIndex;

	Ammo_t		m_AmmoType[MAX_AMMO_TYPES];

	Ammo_t		*GetAmmoOfIndex(int nAmmoIndex);
	int		Index(const char *psz);
	int		PlrDamage(int nAmmoIndex);
	int		NPCDamage(int nAmmoIndex);
	int		MaxCarry(int nAmmoIndex);
	int		DamageType(int nAmmoIndex);
	int		TracerType(int nAmmoIndex);
	float	DamageForce(int nAmmoIndex);
	int		MinSplashSize(int nAmmoIndex);
	int		MaxSplashSize(int nAmmoIndex);
	int		AttackWeapon(int nAmmoIndex);
	int		Flags(int nAmmoIndex);
	float	DmgFactorForClass(int nAmmoIndex, int classNum);

	void		AddAmmoType(char const* name, int weaponNumber, int damageType, int tracerType, int plr_dmg, int npc_dmg, int carry, float physicsForceImpulse, int nFlags, int minSplashSize = 4, int maxSplashSize = 8 );
	void		AddAmmoType(char const* name, int weaponNumber, int damageType, int tracerType, char const* plr_cvar, char const* npc_var, char const* carry_cvar, float physicsForceImpulse, int nFlags, int minSplashSize = 4, int maxSplashSize = 8 );

	CAmmoDef(void);
	virtual ~CAmmoDef( void );

private:
	bool		AddAmmoType(char const* name, int weaponNumber, int damageType, int tracerType, int nFlags, int minSplashSize, int maxSplashSize );

	float dmg_factors[WPN_MAX][NUM_CLASSES];
};


// Get the global ammodef object. This is usually implemented in each mod's game rules file somewhere,
// so the mod can setup custom ammo types.
CAmmoDef* GetAmmoDef();


#endif // AI_AMMODEF_H
 
