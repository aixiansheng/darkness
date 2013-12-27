//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//

#ifndef HL2MP_WEAPON_DRONESLASH_H
#define HL2MP_WEAPON_DRONESLASH_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#ifdef CLIENT_DLL
#define CWeaponDroneSlash C_WeaponDroneSlash
#endif



#define DRONE_SPIT_TIME		1.0f
#define DRONE_SPIT_CTX		"drone_spit_ctx"



class CWeaponDroneSlash : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponDroneSlash, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponDroneSlash();

	float		GetRange(void);
	float		GetFireRate(void);

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);
	//virtual void PrimaryAttack(void);
	virtual void SecondaryAttack(void);
	virtual void ItemPostFrame(void);
	virtual bool ShouldDraw(void) { return false; }

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );

	void RechargeThink(void);

	int charging;

	// Animation event
#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void DisplayUsageHudHint(void);
#endif

	CWeaponDroneSlash( const CWeaponDroneSlash & );

private:
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
};


#endif // HL2MP_WEAPON_DRONESLASH_H

