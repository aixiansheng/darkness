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

#ifndef HL2MP_WEAPON_STINGER_FIRE_H
#define HL2MP_WEAPON_STINGER_FIRE_H
#pragma once


#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL

#include "dlight.h"
#include "fx_line.h"
#include "iefx.h"

#define CWeaponStingerFire C_WeaponStingerFire
#define CStingerFire C_StingerFire

#endif

//-----------------------------------------------------------------------------
// CWeaponStingerFire
//-----------------------------------------------------------------------------

class CWeaponStingerFire : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponStingerFire, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponStingerFire();

	float		GetRange( void );
	float		GetFireRate( void );

	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack(void);
	void		PrimaryAttack(void);
	virtual void	ItemPostFrame(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );
	virtual bool ShouldDraw(void) { return false; }

	CWeaponStingerFire( const CWeaponStingerFire & );

	void RechargeThink(void);
	
	int charging;

private:

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
};


class CStingerFire : public CBaseCombatCharacter {
public:

	DECLARE_CLASS( CStingerFire, CBaseCombatCharacter );
	DECLARE_NETWORKCLASS();
	
	CStingerFire();
	~CStingerFire();

#ifndef CLIENT_DLL
	
	DECLARE_DATADESC();

	void	Spawn( void );
	void	Precache( void );
	void	FlameTouch( CBaseEntity *pOther );

	void	BurnThink(void);
	
	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	CHandle<CWeaponStingerFire>		m_hOwner;

	static CStingerFire *Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner );

protected:
	void CreateSprite(void);

	float m_flMarkDeadTime;
	float m_flDamage;

	float scale;

#else

public:

	virtual void Simulate(void);
	virtual bool ShouldInterpolate(void);

#endif

};


#endif // HL2MP_WEAPON_STINGER_FIRE_H

