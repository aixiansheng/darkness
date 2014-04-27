//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base combat character with no AI
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ammodef.h"
#include "class_info.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Return a pointer to the Ammo at the Index passed in
//-----------------------------------------------------------------------------
Ammo_t *CAmmoDef::GetAmmoOfIndex(int nAmmoIndex)
{
	if ( nAmmoIndex >= m_nAmmoIndex )
		return NULL;

	return &m_AmmoType[ nAmmoIndex ];
}

int CAmmoDef::AttackWeapon(int nAmmoIndex) {
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return WPN_DEFAULT;

	return m_AmmoType[nAmmoIndex].weapon_number;
}

float CAmmoDef::DmgFactorForClass(int nAmmoIndex, int classNum) {
	int weapon_number;

	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return dmg_factors[0][classNum];

	weapon_number = m_AmmoType[nAmmoIndex].weapon_number;

	return dmg_factors[weapon_number][classNum];
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CAmmoDef::Index(const char *psz)
{
	int i;

	if (!psz)
		return -1;

	for (i = 1; i < m_nAmmoIndex; i++)
	{
		if (stricmp( psz, m_AmmoType[i].pName ) == 0)
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int	CAmmoDef::PlrDamage(int nAmmoIndex)
{
	if ( nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex )
		return 0;

	if ( m_AmmoType[nAmmoIndex].pPlrDmg == USE_CVAR )
	{
		if ( m_AmmoType[nAmmoIndex].pPlrDmgCVar )
		{
			return m_AmmoType[nAmmoIndex].pPlrDmgCVar->GetFloat();
		}

		return 0;
	}
	else
	{
		return m_AmmoType[nAmmoIndex].pPlrDmg;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int	CAmmoDef::NPCDamage(int nAmmoIndex)
{
	if ( nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex )
		return 0;

	if ( m_AmmoType[nAmmoIndex].pNPCDmg == USE_CVAR )
	{
		if ( m_AmmoType[nAmmoIndex].pNPCDmgCVar )
		{
			return m_AmmoType[nAmmoIndex].pNPCDmgCVar->GetFloat();
		}

		return 0;
	}
	else
	{
		return m_AmmoType[nAmmoIndex].pNPCDmg;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int	CAmmoDef::MaxCarry(int nAmmoIndex)
{
	if ( nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex )
		return 0;

	if ( m_AmmoType[nAmmoIndex].pMaxCarry == USE_CVAR )
	{
		if ( m_AmmoType[nAmmoIndex].pMaxCarryCVar )
			return m_AmmoType[nAmmoIndex].pMaxCarryCVar->GetFloat();

		return 0;
	}
	else
	{
		return m_AmmoType[nAmmoIndex].pMaxCarry;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int	CAmmoDef::DamageType(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 0;

	return m_AmmoType[nAmmoIndex].nDamageType;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CAmmoDef::Flags(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 0;

	return m_AmmoType[nAmmoIndex].nFlags;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int	CAmmoDef::MinSplashSize(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 4;

	return m_AmmoType[nAmmoIndex].nMinSplashSize;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int	CAmmoDef::MaxSplashSize(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 8;

	return m_AmmoType[nAmmoIndex].nMaxSplashSize;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int	CAmmoDef::TracerType(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 0;

	return m_AmmoType[nAmmoIndex].eTracerType;
}

float CAmmoDef::DamageForce(int nAmmoIndex)
{
	if ( nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex )
		return 0;

	return m_AmmoType[nAmmoIndex].physicsForceImpulse;
}

//-----------------------------------------------------------------------------
// Purpose: Create an Ammo type with the name, decal, and tracer.
// Does not increment m_nAmmoIndex because the functions below do so and 
//  are the only entry point.
//-----------------------------------------------------------------------------
bool CAmmoDef::AddAmmoType(char const* name, int weaponNumber, int damageType, int tracerType, int nFlags, int minSplashSize, int maxSplashSize )
{
	if (m_nAmmoIndex == MAX_AMMO_TYPES)
		return false;

	int len = strlen(name);
	m_AmmoType[m_nAmmoIndex].pName = new char[len+1];
	Q_strncpy(m_AmmoType[m_nAmmoIndex].pName, name,len+1);
	m_AmmoType[m_nAmmoIndex].nDamageType	= damageType;
	m_AmmoType[m_nAmmoIndex].eTracerType	= tracerType;
	m_AmmoType[m_nAmmoIndex].nMinSplashSize	= minSplashSize;
	m_AmmoType[m_nAmmoIndex].nMaxSplashSize	= maxSplashSize;
	m_AmmoType[m_nAmmoIndex].nFlags	= nFlags;
	m_AmmoType[m_nAmmoIndex].weapon_number = weaponNumber;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Add an ammo type with it's damage & carrying capability specified via cvars
//-----------------------------------------------------------------------------
void CAmmoDef::AddAmmoType(char const* name, int weaponNumber, int damageType, int tracerType, 
	char const* plr_cvar, char const* npc_cvar, char const* carry_cvar, 
	float physicsForceImpulse, int nFlags, int minSplashSize, int maxSplashSize)
{
	if ( AddAmmoType( name, weaponNumber, damageType, tracerType, nFlags, minSplashSize, maxSplashSize ) == false )
		return;

	if (plr_cvar)
	{
		m_AmmoType[m_nAmmoIndex].pPlrDmgCVar	= cvar->FindVar(plr_cvar);
		if (!m_AmmoType[m_nAmmoIndex].pPlrDmgCVar)
		{
			Msg("ERROR: Ammo (%s) found no CVar named (%s)\n",name,plr_cvar);
		}
		m_AmmoType[m_nAmmoIndex].pPlrDmg = USE_CVAR;
	}
	if (npc_cvar)
	{
		m_AmmoType[m_nAmmoIndex].pNPCDmgCVar	= cvar->FindVar(npc_cvar);
		if (!m_AmmoType[m_nAmmoIndex].pNPCDmgCVar)
		{
			Msg("ERROR: Ammo (%s) found no CVar named (%s)\n",name,npc_cvar);
		}
		m_AmmoType[m_nAmmoIndex].pNPCDmg = USE_CVAR;
	}
	if (carry_cvar)
	{
		m_AmmoType[m_nAmmoIndex].pMaxCarryCVar= cvar->FindVar(carry_cvar);
		if (!m_AmmoType[m_nAmmoIndex].pMaxCarryCVar)
		{
			Msg("ERROR: Ammo (%s) found no CVar named (%s)\n",name,carry_cvar);
		}
		m_AmmoType[m_nAmmoIndex].pMaxCarry = USE_CVAR;
	}
	m_AmmoType[m_nAmmoIndex].physicsForceImpulse = physicsForceImpulse;
	m_nAmmoIndex++;
}

//-----------------------------------------------------------------------------
// Purpose: Add an ammo type with it's damage & carrying capability specified via integers
//-----------------------------------------------------------------------------
void CAmmoDef::AddAmmoType(char const* name, int weaponNumber, int damageType, int tracerType, 
	int plr_dmg, int npc_dmg, int carry, float physicsForceImpulse, 
	int nFlags, int minSplashSize, int maxSplashSize )
{
	if ( AddAmmoType( name, weaponNumber, damageType, tracerType, nFlags, minSplashSize, maxSplashSize ) == false )
		return;

	m_AmmoType[m_nAmmoIndex].pPlrDmg = plr_dmg;
	m_AmmoType[m_nAmmoIndex].pNPCDmg = npc_dmg;
	m_AmmoType[m_nAmmoIndex].pMaxCarry = carry;
	m_AmmoType[m_nAmmoIndex].physicsForceImpulse = physicsForceImpulse;

	m_nAmmoIndex++;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CAmmoDef::CAmmoDef(void)
{
	// Start with an index of 1.  Client assumes 0 is an invalid ammo type
	m_nAmmoIndex = 1;
	memset( m_AmmoType, 0, sizeof( m_AmmoType ) );

	dmg_factors[WPN_DEFAULT][CLASS_ENGINEER_IDX]		= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_GRUNT_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_SHOCK_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_HEAVY_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_COMMANDO_IDX]		= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_EXTERMINATOR_IDX]	= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_MECH_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_BREEDER_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_HATCHY_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_DRONE_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_KAMI_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_STINGER_IDX]			= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_GUARDIAN_IDX]		= -1.0f;
	dmg_factors[WPN_DEFAULT][CLASS_STALKER_IDX]			= -1.0f;


	// ENGY
	dmg_factors[WPN_ENGY][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_ENGY][CLASS_GRUNT_IDX]			= 0.0f;
	dmg_factors[WPN_ENGY][CLASS_SHOCK_IDX]			= 0.0f;
	dmg_factors[WPN_ENGY][CLASS_HEAVY_IDX]			= 0.0f;
	dmg_factors[WPN_ENGY][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_ENGY][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_ENGY][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_ENGY][CLASS_BREEDER_IDX]		= 1.0f;
	dmg_factors[WPN_ENGY][CLASS_HATCHY_IDX]			= 2.0f;
	dmg_factors[WPN_ENGY][CLASS_DRONE_IDX]			= 2.0f;
	dmg_factors[WPN_ENGY][CLASS_KAMI_IDX]			= 2.0f;
	dmg_factors[WPN_ENGY][CLASS_STINGER_IDX]		= 1.0f;
	dmg_factors[WPN_ENGY][CLASS_GUARDIAN_IDX]		= 1.0f;
	dmg_factors[WPN_ENGY][CLASS_STALKER_IDX]		= 1.0f;

	// PISTOL
	dmg_factors[WPN_PISTOL][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_PISTOL][CLASS_GRUNT_IDX]		= 0.0f;
	dmg_factors[WPN_PISTOL][CLASS_SHOCK_IDX]		= 0.0f;
	dmg_factors[WPN_PISTOL][CLASS_HEAVY_IDX]		= 0.0f;
	dmg_factors[WPN_PISTOL][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_PISTOL][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_PISTOL][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_PISTOL][CLASS_BREEDER_IDX]		= 1.0f;
	dmg_factors[WPN_PISTOL][CLASS_HATCHY_IDX]		= 2.0f;
	dmg_factors[WPN_PISTOL][CLASS_DRONE_IDX]		= 1.0f;
	dmg_factors[WPN_PISTOL][CLASS_KAMI_IDX]			= 2.0f;
	dmg_factors[WPN_PISTOL][CLASS_STINGER_IDX]		= 1.0f;
	dmg_factors[WPN_PISTOL][CLASS_GUARDIAN_IDX]		= 1.0f;
	dmg_factors[WPN_PISTOL][CLASS_STALKER_IDX]		= 1.0f;

	// SMG
	dmg_factors[WPN_SMG][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_SMG][CLASS_GRUNT_IDX]			= 0.0f;
	dmg_factors[WPN_SMG][CLASS_SHOCK_IDX]			= 0.0f;
	dmg_factors[WPN_SMG][CLASS_HEAVY_IDX]			= 0.0f;
	dmg_factors[WPN_SMG][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_SMG][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_SMG][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_SMG][CLASS_BREEDER_IDX]			= 0.7f;
	dmg_factors[WPN_SMG][CLASS_HATCHY_IDX]			= 1.2f;
	dmg_factors[WPN_SMG][CLASS_DRONE_IDX]			= 0.7f;
	dmg_factors[WPN_SMG][CLASS_KAMI_IDX]			= 1.2f;
	dmg_factors[WPN_SMG][CLASS_STINGER_IDX]			= 0.7f;
	dmg_factors[WPN_SMG][CLASS_GUARDIAN_IDX]		= 0.7f;
	dmg_factors[WPN_SMG][CLASS_STALKER_IDX]			= 0.65f;

	// SHOTGUN BUCK
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_GRUNT_IDX]			= 0.0f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_SHOCK_IDX]			= 0.0f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_HEAVY_IDX]			= 0.0f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_BREEDER_IDX]		= 1.0f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_HATCHY_IDX]			= 1.2f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_DRONE_IDX]			= 0.9f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_KAMI_IDX]			= 1.1f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_STINGER_IDX]		= 0.9f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_GUARDIAN_IDX]		= 0.9f;
	dmg_factors[WPN_SHOTGUN_BUCK][CLASS_STALKER_IDX]		= 0.9f;

	// SHOTGUN XP
	dmg_factors[WPN_SHOTGUN_XP][CLASS_ENGINEER_IDX]			= 1.2f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_GRUNT_IDX]			= 1.0f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_SHOCK_IDX]			= 1.0f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_HEAVY_IDX]			= 2.0f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_EXTERMINATOR_IDX]		= 0.9f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_MECH_IDX]				= 0.9f;
	
	dmg_factors[WPN_SHOTGUN_XP][CLASS_BREEDER_IDX]			= 1.0f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_HATCHY_IDX]			= 1.1f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_DRONE_IDX]			= 1.1f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_KAMI_IDX]				= 0.8f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_STINGER_IDX]			= 1.0f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_GUARDIAN_IDX]			= 0.7f;
	dmg_factors[WPN_SHOTGUN_XP][CLASS_STALKER_IDX]			= 1.6f;

	// RPG
	dmg_factors[WPN_RPG][CLASS_ENGINEER_IDX]		= 1.0f;
	dmg_factors[WPN_RPG][CLASS_GRUNT_IDX]			= 1.0f;
	dmg_factors[WPN_RPG][CLASS_SHOCK_IDX]			= 1.0f;
	dmg_factors[WPN_RPG][CLASS_HEAVY_IDX]			= 1.0f;
	dmg_factors[WPN_RPG][CLASS_COMMANDO_IDX]		= 1.0f;
	dmg_factors[WPN_RPG][CLASS_EXTERMINATOR_IDX]	= 1.0f;
	dmg_factors[WPN_RPG][CLASS_MECH_IDX]			= 1.0f;
	
	dmg_factors[WPN_RPG][CLASS_BREEDER_IDX]			= 1.0f;
	dmg_factors[WPN_RPG][CLASS_HATCHY_IDX]			= 1.0f;
	dmg_factors[WPN_RPG][CLASS_DRONE_IDX]			= 1.0f;
	dmg_factors[WPN_RPG][CLASS_KAMI_IDX]			= 0.8f;
	dmg_factors[WPN_RPG][CLASS_STINGER_IDX]			= 1.0f;
	dmg_factors[WPN_RPG][CLASS_GUARDIAN_IDX]		= 0.7f;
	dmg_factors[WPN_RPG][CLASS_STALKER_IDX]			= 1.3f;

	// SILENCED SMG
	dmg_factors[WPN_SILENCED_SMG][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_GRUNT_IDX]			= 0.0f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_SHOCK_IDX]			= 0.0f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_HEAVY_IDX]			= 0.0f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_SILENCED_SMG][CLASS_BREEDER_IDX]		= 0.8f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_HATCHY_IDX]			= 0.8f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_DRONE_IDX]			= 0.75f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_KAMI_IDX]			= 0.8f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_STINGER_IDX]		= 0.6f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_GUARDIAN_IDX]		= 0.6f;
	dmg_factors[WPN_SILENCED_SMG][CLASS_STALKER_IDX]		= 0.6f;

	// 357 MAGNUM
	dmg_factors[WPN_357][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_357][CLASS_GRUNT_IDX]			= 0.0f;
	dmg_factors[WPN_357][CLASS_SHOCK_IDX]			= 0.0f;
	dmg_factors[WPN_357][CLASS_HEAVY_IDX]			= 0.0f;
	dmg_factors[WPN_357][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_357][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_357][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_357][CLASS_BREEDER_IDX]			= 1.0f;
	dmg_factors[WPN_357][CLASS_HATCHY_IDX]			= 1.0f;
	dmg_factors[WPN_357][CLASS_DRONE_IDX]			= 1.0f;
	dmg_factors[WPN_357][CLASS_KAMI_IDX]			= 1.0f;
	dmg_factors[WPN_357][CLASS_STINGER_IDX]			= 0.7f;
	dmg_factors[WPN_357][CLASS_GUARDIAN_IDX]		= 0.7f;
	dmg_factors[WPN_357][CLASS_STALKER_IDX]			= 1.1f;

	// C4
	dmg_factors[WPN_C4][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_GRUNT_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_SHOCK_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_HEAVY_IDX]			= 1.1f;
	dmg_factors[WPN_C4][CLASS_COMMANDO_IDX]			= 1.4f;
	dmg_factors[WPN_C4][CLASS_EXTERMINATOR_IDX]		= 1.1f;
	dmg_factors[WPN_C4][CLASS_MECH_IDX]				= 1.2f;
	
	dmg_factors[WPN_C4][CLASS_BREEDER_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_HATCHY_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_DRONE_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_KAMI_IDX]				= 1.0f;
	dmg_factors[WPN_C4][CLASS_STINGER_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_GUARDIAN_IDX]			= 1.0f;
	dmg_factors[WPN_C4][CLASS_STALKER_IDX]			= 1.0f;

	// FRAG GRENADE
	dmg_factors[WPN_FRAG][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_GRUNT_IDX]				= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_SHOCK_IDX]				= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_HEAVY_IDX]				= 1.1f;
	dmg_factors[WPN_FRAG][CLASS_COMMANDO_IDX]			= 1.4f;
	dmg_factors[WPN_FRAG][CLASS_EXTERMINATOR_IDX]		= 1.1f;
	dmg_factors[WPN_FRAG][CLASS_MECH_IDX]				= 1.2f;
	
	dmg_factors[WPN_FRAG][CLASS_BREEDER_IDX]			= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_HATCHY_IDX]				= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_DRONE_IDX]				= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_KAMI_IDX]				= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_STINGER_IDX]			= 1.1f;
	dmg_factors[WPN_FRAG][CLASS_GUARDIAN_IDX]			= 1.0f;
	dmg_factors[WPN_FRAG][CLASS_STALKER_IDX]			= 1.2f;

	// PLASMA RIFLE
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_ENGINEER_IDX]			= 0.0f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_GRUNT_IDX]				= 0.0f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_SHOCK_IDX]				= 0.0f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_HEAVY_IDX]				= 0.0f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_COMMANDO_IDX]			= 0.0f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_EXTERMINATOR_IDX]		= 0.0f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_MECH_IDX]				= 0.0f;
	
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_BREEDER_IDX]			= 0.5f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_HATCHY_IDX]				= 0.3f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_DRONE_IDX]				= 0.5f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_KAMI_IDX]				= 0.4f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_STINGER_IDX]			= 0.4f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_GUARDIAN_IDX]			= 1.2f;
	dmg_factors[WPN_PLASMA_RIFLE][CLASS_STALKER_IDX]			= 1.0f;

	// RAILGUN
	dmg_factors[WPN_RAILGUN][CLASS_ENGINEER_IDX]			= 0.0f;
	dmg_factors[WPN_RAILGUN][CLASS_GRUNT_IDX]				= 0.0f;
	dmg_factors[WPN_RAILGUN][CLASS_SHOCK_IDX]				= 0.0f;
	dmg_factors[WPN_RAILGUN][CLASS_HEAVY_IDX]				= 0.0f;
	dmg_factors[WPN_RAILGUN][CLASS_COMMANDO_IDX]			= 0.0f;
	dmg_factors[WPN_RAILGUN][CLASS_EXTERMINATOR_IDX]		= 0.0f;
	dmg_factors[WPN_RAILGUN][CLASS_MECH_IDX]				= 0.0f;
	
	dmg_factors[WPN_RAILGUN][CLASS_BREEDER_IDX]				= 1.0f;
	dmg_factors[WPN_RAILGUN][CLASS_HATCHY_IDX]				= 1.0f;
	dmg_factors[WPN_RAILGUN][CLASS_DRONE_IDX]				= 1.0f;
	dmg_factors[WPN_RAILGUN][CLASS_KAMI_IDX]				= 1.0f;
	dmg_factors[WPN_RAILGUN][CLASS_STINGER_IDX]				= 0.8f;
	dmg_factors[WPN_RAILGUN][CLASS_GUARDIAN_IDX]			= 1.2f;
	dmg_factors[WPN_RAILGUN][CLASS_STALKER_IDX]				= 1.0f;

	// PLASMA CANON
	dmg_factors[WPN_PLASMA_CANON][CLASS_ENGINEER_IDX]			= 0.0f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_GRUNT_IDX]				= 0.0f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_SHOCK_IDX]				= 0.0f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_HEAVY_IDX]				= 0.0f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_COMMANDO_IDX]			= 0.0f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_EXTERMINATOR_IDX]		= 0.0f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_MECH_IDX]				= 0.0f;
	
	dmg_factors[WPN_PLASMA_CANON][CLASS_BREEDER_IDX]			= 0.6f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_HATCHY_IDX]				= 0.7f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_DRONE_IDX]				= 0.7f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_KAMI_IDX]				= 0.7f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_STINGER_IDX]			= 0.8f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_GUARDIAN_IDX]			= 1.0f;
	dmg_factors[WPN_PLASMA_CANON][CLASS_STALKER_IDX]			= 0.8f;

	// HATCHY SLASH
	dmg_factors[WPN_HATCHY_SLASH][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_GRUNT_IDX]				= 1.3f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_SHOCK_IDX]				= 1.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_HEAVY_IDX]				= 1.2f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_EXTERMINATOR_IDX]		= 0.8f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_MECH_IDX]				= 1.2f;
	
	dmg_factors[WPN_HATCHY_SLASH][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_HATCHY_SLASH][CLASS_STALKER_IDX]			= 0.0f;

	// DRONE SLASH
	dmg_factors[WPN_DRONE_SLASH][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_GRUNT_IDX]				= 1.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_SHOCK_IDX]				= 0.7f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_HEAVY_IDX]				= 1.1f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_COMMANDO_IDX]			= 1.1f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_EXTERMINATOR_IDX]		= 0.7f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_MECH_IDX]				= 1.2f;
	
	dmg_factors[WPN_DRONE_SLASH][CLASS_BREEDER_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_STINGER_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_DRONE_SLASH][CLASS_STALKER_IDX]				= 0.0f;

	// DRONE SPIT
	dmg_factors[WPN_DRONE_SPIT][CLASS_ENGINEER_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_GRUNT_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_SHOCK_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_HEAVY_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_COMMANDO_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_EXTERMINATOR_IDX]			= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_MECH_IDX]					= 0.0f;
	
	dmg_factors[WPN_DRONE_SPIT][CLASS_BREEDER_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_KAMI_IDX]					= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_STINGER_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_GUARDIAN_IDX]				= 0.0f;
	dmg_factors[WPN_DRONE_SPIT][CLASS_STALKER_IDX]				= 0.0f;

	// KAMI SLASH
	dmg_factors[WPN_KAMI_SLASH][CLASS_ENGINEER_IDX]				= 1.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_GRUNT_IDX]				= 1.3f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_SHOCK_IDX]				= 1.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_HEAVY_IDX]				= 1.2f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_COMMANDO_IDX]				= 1.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_EXTERMINATOR_IDX]			= 0.8f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_MECH_IDX]					= 1.2f;
	
	dmg_factors[WPN_KAMI_SLASH][CLASS_BREEDER_IDX]				= 0.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_KAMI_IDX]					= 0.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_STINGER_IDX]				= 0.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_GUARDIAN_IDX]				= 0.0f;
	dmg_factors[WPN_KAMI_SLASH][CLASS_STALKER_IDX]				= 0.0f;

	// KAMI EXPLOSION
	dmg_factors[WPN_KAMI_XP][CLASS_ENGINEER_IDX]				= 1.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_GRUNT_IDX]					= 1.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_SHOCK_IDX]					= 1.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_HEAVY_IDX]					= 1.2f;
	dmg_factors[WPN_KAMI_XP][CLASS_COMMANDO_IDX]				= 1.2f;
	dmg_factors[WPN_KAMI_XP][CLASS_EXTERMINATOR_IDX]			= 1.2f;
	dmg_factors[WPN_KAMI_XP][CLASS_MECH_IDX]					= 1.0f;
	
	dmg_factors[WPN_KAMI_XP][CLASS_BREEDER_IDX]					= 0.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_HATCHY_IDX]					= 0.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_DRONE_IDX]					= 0.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_KAMI_IDX]					= 0.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_STINGER_IDX]					= 0.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_GUARDIAN_IDX]				= 0.0f;
	dmg_factors[WPN_KAMI_XP][CLASS_STALKER_IDX]					= 0.0f;

	// STINGER SLASH
	dmg_factors[WPN_STINGER_SLASH][CLASS_ENGINEER_IDX]			= 0.8f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_GRUNT_IDX]				= 1.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_SHOCK_IDX]				= 0.65f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_HEAVY_IDX]				= 1.1f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_EXTERMINATOR_IDX]		= 0.65f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_MECH_IDX]				= 1.4f;
	
	dmg_factors[WPN_STINGER_SLASH][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_STINGER_SLASH][CLASS_STALKER_IDX]			= 0.0f;

	// STINGER FIRE
	dmg_factors[WPN_STINGER_FIRE][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_GRUNT_IDX]				= 1.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_SHOCK_IDX]				= 1.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_HEAVY_IDX]				= 1.2f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_EXTERMINATOR_IDX]		= 0.65f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_MECH_IDX]				= 1.0f;
	
	dmg_factors[WPN_STINGER_FIRE][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_STINGER_FIRE][CLASS_STALKER_IDX]			= 0.0f;

	// ACID	GREN
	dmg_factors[WPN_ACID_GREN][CLASS_ENGINEER_IDX]				= 1.1f;
	dmg_factors[WPN_ACID_GREN][CLASS_GRUNT_IDX]					= 0.8f;
	dmg_factors[WPN_ACID_GREN][CLASS_SHOCK_IDX]					= 0.8f;
	dmg_factors[WPN_ACID_GREN][CLASS_HEAVY_IDX]					= 1.7f;
	dmg_factors[WPN_ACID_GREN][CLASS_COMMANDO_IDX]				= 1.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_EXTERMINATOR_IDX]			= 0.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_MECH_IDX]					= 0.0f;
	
	dmg_factors[WPN_ACID_GREN][CLASS_BREEDER_IDX]				= 0.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_DRONE_IDX]					= 0.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_KAMI_IDX]					= 0.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_STINGER_IDX]				= 0.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_GUARDIAN_IDX]				= 0.0f;
	dmg_factors[WPN_ACID_GREN][CLASS_STALKER_IDX]				= 0.0f;

	// GUARDIAN SLASH
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_ENGINEER_IDX]			= 0.6f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_GRUNT_IDX]			= 0.7f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_SHOCK_IDX]			= 0.7f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_HEAVY_IDX]			= 1.1f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_EXTERMINATOR_IDX]		= 1.3f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_MECH_IDX]				= 1.0f;
	
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_DRONE_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SLASH][CLASS_STALKER_IDX]			= 0.0f;

	// GUARDIAN SPIKE
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_GRUNT_IDX]			= 1.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_SHOCK_IDX]			= 1.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_HEAVY_IDX]			= 1.3f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_EXTERMINATOR_IDX]		= 1.3f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_MECH_IDX]				= 1.1f;
	
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_DRONE_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_GUARDIAN_SPIKE][CLASS_STALKER_IDX]			= 0.0f;

	// STALKER SLASH
	dmg_factors[WPN_STALKER_SLASH][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_GRUNT_IDX]				= 1.1f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_SHOCK_IDX]				= 0.9f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_HEAVY_IDX]				= 1.1f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_COMMANDO_IDX]			= 0.8f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_EXTERMINATOR_IDX]		= 0.8f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_MECH_IDX]				= 0.8f;
	
	dmg_factors[WPN_STALKER_SLASH][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SLASH][CLASS_STALKER_IDX]			= 0.0f;

	// STALKER GREN
	dmg_factors[WPN_STALKER_GREN][CLASS_ENGINEER_IDX]			= 1.1f;
	dmg_factors[WPN_STALKER_GREN][CLASS_GRUNT_IDX]				= 1.1f;
	dmg_factors[WPN_STALKER_GREN][CLASS_SHOCK_IDX]				= 1.0f;
	dmg_factors[WPN_STALKER_GREN][CLASS_HEAVY_IDX]				= 1.3f;
	dmg_factors[WPN_STALKER_GREN][CLASS_COMMANDO_IDX]			= 1.2f;
	dmg_factors[WPN_STALKER_GREN][CLASS_EXTERMINATOR_IDX]		= 1.4f;
	dmg_factors[WPN_STALKER_GREN][CLASS_MECH_IDX]				= 1.1f;
	
	dmg_factors[WPN_STALKER_GREN][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_GREN][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_STALKER_GREN][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_STALKER_GREN][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_STALKER_GREN][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_GREN][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_GREN][CLASS_STALKER_IDX]			= 0.0f;

	// STALKER SPIKE
	dmg_factors[WPN_STALKER_SPIKE][CLASS_ENGINEER_IDX]			= 1.1f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_GRUNT_IDX]				= 1.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_SHOCK_IDX]				= 1.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_HEAVY_IDX]				= 1.1f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_EXTERMINATOR_IDX]		= 0.6f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_MECH_IDX]				= 1.1f;
	
	dmg_factors[WPN_STALKER_SPIKE][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_STALKER_SPIKE][CLASS_STALKER_IDX]			= 0.0f;

	
	// OBSTACLE
	dmg_factors[WPN_OBSTACLE][CLASS_ENGINEER_IDX]			= 1.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_GRUNT_IDX]				= 1.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_SHOCK_IDX]				= 1.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_HEAVY_IDX]				= 1.1f;
	dmg_factors[WPN_OBSTACLE][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_EXTERMINATOR_IDX]		= 1.1f;
	dmg_factors[WPN_OBSTACLE][CLASS_MECH_IDX]				= 1.4f;
	
	dmg_factors[WPN_OBSTACLE][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_HATCHY_IDX]				= 0.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_DRONE_IDX]				= 0.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_OBSTACLE][CLASS_STALKER_IDX]			= 0.0f;

	// SPIKER
	dmg_factors[WPN_SPIKER][CLASS_ENGINEER_IDX]			= 1.1f;
	dmg_factors[WPN_SPIKER][CLASS_GRUNT_IDX]			= 0.8f;
	dmg_factors[WPN_SPIKER][CLASS_SHOCK_IDX]			= 0.8f;
	dmg_factors[WPN_SPIKER][CLASS_HEAVY_IDX]			= 1.0f;
	dmg_factors[WPN_SPIKER][CLASS_COMMANDO_IDX]			= 0.7f;
	dmg_factors[WPN_SPIKER][CLASS_EXTERMINATOR_IDX]		= 1.0f;
	dmg_factors[WPN_SPIKER][CLASS_MECH_IDX]				= 0.8f;
	
	dmg_factors[WPN_SPIKER][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_SPIKER][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_SPIKER][CLASS_DRONE_IDX]			= 0.0f;
	dmg_factors[WPN_SPIKER][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_SPIKER][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_SPIKER][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_SPIKER][CLASS_STALKER_IDX]			= 0.0f;

	// GASSER
	dmg_factors[WPN_GASSER][CLASS_ENGINEER_IDX]			= 1.1f;
	dmg_factors[WPN_GASSER][CLASS_GRUNT_IDX]			= 1.1f;
	dmg_factors[WPN_GASSER][CLASS_SHOCK_IDX]			= 0.9f;
	dmg_factors[WPN_GASSER][CLASS_HEAVY_IDX]			= 1.1f;
	dmg_factors[WPN_GASSER][CLASS_COMMANDO_IDX]			= 1.0f;
	dmg_factors[WPN_GASSER][CLASS_EXTERMINATOR_IDX]		= 0.0f;
	dmg_factors[WPN_GASSER][CLASS_MECH_IDX]				= 0.0f;
	
	dmg_factors[WPN_GASSER][CLASS_BREEDER_IDX]			= 0.0f;
	dmg_factors[WPN_GASSER][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_GASSER][CLASS_DRONE_IDX]			= 0.0f;
	dmg_factors[WPN_GASSER][CLASS_KAMI_IDX]				= 0.0f;
	dmg_factors[WPN_GASSER][CLASS_STINGER_IDX]			= 0.0f;
	dmg_factors[WPN_GASSER][CLASS_GUARDIAN_IDX]			= 0.0f;
	dmg_factors[WPN_GASSER][CLASS_STALKER_IDX]			= 0.0f;

	// EXPLOSIVE MINE
	dmg_factors[WPN_XP_MINE][CLASS_ENGINEER_IDX]		= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_GRUNT_IDX]			= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_SHOCK_IDX]			= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_HEAVY_IDX]			= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_COMMANDO_IDX]		= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_EXTERMINATOR_IDX]	= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_MECH_IDX]			= 1.0f;
	
	dmg_factors[WPN_XP_MINE][CLASS_BREEDER_IDX]			= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_HATCHY_IDX]			= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_DRONE_IDX]			= 1.0f;
	dmg_factors[WPN_XP_MINE][CLASS_KAMI_IDX]			= 0.8f;
	dmg_factors[WPN_XP_MINE][CLASS_STINGER_IDX]			= 1.1f;
	dmg_factors[WPN_XP_MINE][CLASS_GUARDIAN_IDX]		= 0.7f;
	dmg_factors[WPN_XP_MINE][CLASS_STALKER_IDX]			= 1.2f;

	// MACHINEGUN TURRET
	dmg_factors[WPN_MG_TURRET][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_GRUNT_IDX]			= 0.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_SHOCK_IDX]			= 0.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_HEAVY_IDX]			= 0.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_MG_TURRET][CLASS_BREEDER_IDX]		= 1.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_HATCHY_IDX]		= 1.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_DRONE_IDX]			= 1.0f;
	dmg_factors[WPN_MG_TURRET][CLASS_KAMI_IDX]			= 0.9f;
	dmg_factors[WPN_MG_TURRET][CLASS_STINGER_IDX]		= 0.8f;
	dmg_factors[WPN_MG_TURRET][CLASS_GUARDIAN_IDX]		= 0.8f;
	dmg_factors[WPN_MG_TURRET][CLASS_STALKER_IDX]		= 0.8f;

	// MISSILE TURRET
	dmg_factors[WPN_MSL_TURRET][CLASS_ENGINEER_IDX]		= 0.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_GRUNT_IDX]		= 0.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_SHOCK_IDX]		= 0.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_HEAVY_IDX]		= 0.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_COMMANDO_IDX]		= 0.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_MSL_TURRET][CLASS_BREEDER_IDX]		= 1.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_HATCHY_IDX]		= 1.5f;
	dmg_factors[WPN_MSL_TURRET][CLASS_DRONE_IDX]		= 1.0f;
	dmg_factors[WPN_MSL_TURRET][CLASS_KAMI_IDX]			= 1.5f;
	dmg_factors[WPN_MSL_TURRET][CLASS_STINGER_IDX]		= 0.9f;
	dmg_factors[WPN_MSL_TURRET][CLASS_GUARDIAN_IDX]		= 1.3f;
	dmg_factors[WPN_MSL_TURRET][CLASS_STALKER_IDX]		= 1.1f;

	// INFESTED CORPSE
	dmg_factors[WPN_INFESTED][CLASS_ENGINEER_IDX]		= 1.0f;
	dmg_factors[WPN_INFESTED][CLASS_GRUNT_IDX]			= 1.2f;
	dmg_factors[WPN_INFESTED][CLASS_SHOCK_IDX]			= 0.9f;
	dmg_factors[WPN_INFESTED][CLASS_HEAVY_IDX]			= 1.2f;
	dmg_factors[WPN_INFESTED][CLASS_COMMANDO_IDX]		= 1.1f;
	dmg_factors[WPN_INFESTED][CLASS_EXTERMINATOR_IDX]	= 0.0f;
	dmg_factors[WPN_INFESTED][CLASS_MECH_IDX]			= 0.0f;
	
	dmg_factors[WPN_INFESTED][CLASS_BREEDER_IDX]		= 0.0f;
	dmg_factors[WPN_INFESTED][CLASS_HATCHY_IDX]			= 0.0f;
	dmg_factors[WPN_INFESTED][CLASS_DRONE_IDX]			= 0.0f;
	dmg_factors[WPN_INFESTED][CLASS_KAMI_IDX]			= 0.0f;
	dmg_factors[WPN_INFESTED][CLASS_STINGER_IDX]		= 0.0f;
	dmg_factors[WPN_INFESTED][CLASS_GUARDIAN_IDX]		= 0.0f;
	dmg_factors[WPN_INFESTED][CLASS_STALKER_IDX]		= 0.0f;

}

CAmmoDef::~CAmmoDef( void )
{
	for ( int i = 1; i < MAX_AMMO_TYPES; i++ )
	{
		delete[] m_AmmoType[ i ].pName;
	}
}


