//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2MP_WEAPON_GUARDIAN_SLASH_H
#define HL2MP_WEAPON_GUARDIAN_SLASH_H
#pragma once


#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL
#define CWeaponGuardianSlash C_WeaponGuardianSlash
#endif

//-----------------------------------------------------------------------------
// CWeaponGuardianSlash
//-----------------------------------------------------------------------------

class CWeaponGuardianSlash : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponGuardianSlash, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponGuardianSlash();

	float		GetRange( void );
	float		GetFireRate( void );

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack( void )	{	return;	}

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );
	virtual bool ShouldDraw(void) { return false; }

#ifndef CLIENT_DLL

	virtual void Spawn(void);
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	int WeaponMeleeAttack1Condition( float flDot, float flDist );
	virtual void PrimaryAttack( void );

#endif

	CWeaponGuardianSlash( const CWeaponGuardianSlash & );

private:
		
};


#endif // HL2MP_WEAPON_GUARDIAN_SLASH_H

