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

#ifndef HL2MP_WEAPON_STALKER_SLASH_H
#define HL2MP_WEAPON_STALKER_SLASH_H
#pragma once


#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL
#define CWeaponStalkerSlash C_WeaponStalkerSlash
#endif

//-----------------------------------------------------------------------------
// CWeaponStalkerSlash
//-----------------------------------------------------------------------------

class CWeaponStalkerSlash : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponStalkerSlash, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponStalkerSlash();

	float		GetRange( void );
	float		GetFireRate( void );

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack(void);
	void		PrimaryAttack(void);
	virtual void ItemPostFrame(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );

	virtual bool ShouldDraw(void) { return false; }

	// Animation event
#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	int WeaponMeleeAttack1Condition( float flDot, float flDist );

	void RechargeThink(void);
	void ResetRecharge(void);

	float m_flNextRecharge;

#endif

	CWeaponStalkerSlash( const CWeaponStalkerSlash & );

private:
		
};


#endif // HL2MP_WEAPON_STALKER_SLASH_H

