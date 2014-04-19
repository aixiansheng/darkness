//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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

#ifndef HL2MP_WEAPON_PLASMA_CANON_H
#define HL2MP_WEAPON_PLASMA_CANON_H
#pragma once


#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL

#include "dlight.h"
#include "fx_line.h"
#include "iefx.h"

#define CWeaponPlasmaCanon C_WeaponPlasmaCanon
#define CPlasmaBolt C_PlasmaBolt

#else

#include "Sprite.h"

#endif

//-----------------------------------------------------------------------------
// CWeaponPlasmaCanon
//-----------------------------------------------------------------------------

class CWeaponPlasmaCanon : public CBaseHL2MPBludgeonWeapon {
public:
	DECLARE_CLASS( CWeaponPlasmaCanon, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponPlasmaCanon();

	float		GetRange( void );
	float		GetFireRate( void );
	float		GetDamageForActivity( Activity hitActivity );
	void		FireCanon(bool left);
	virtual void	ItemPostFrame(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );
	virtual bool ShouldDraw(void) { return false; }

	CWeaponPlasmaCanon( const CWeaponPlasmaCanon & );

private:

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
};


class CPlasmaBolt : public CBaseCombatCharacter {
public:

	DECLARE_CLASS( CPlasmaBolt, CBaseCombatCharacter );
	DECLARE_NETWORKCLASS();

	CPlasmaBolt();
	~CPlasmaBolt();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
	void	Spawn( void );
	void	Precache( void );

	void	BoltTouch( CBaseEntity *pOther );
	void	BurnOutThink(void);
	
	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	CHandle<CWeaponPlasmaCanon>		m_hOwner;

	static CPlasmaBolt *Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner );

protected:
	void CreateSprite(void);

	float m_flDamage;

	float scale;

	int ammo_type;

#else

public:

	virtual void Simulate(void);
	virtual bool ShouldInterpolate(void);

#endif

};


#endif // HL2MP_WEAPON_PLASMA_CANON_H

