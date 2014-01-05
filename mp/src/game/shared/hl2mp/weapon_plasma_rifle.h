#ifndef HL2MP_WEAPON_PLASMA_RIFLE_H
#define HL2MP_WEAPON_PLASMA_RIFLE_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbase_machinegun.h"


#ifdef CLIENT_DLL
#define CWeaponPlasmaRifle C_WeaponPlasmaRifle
#endif

#define	PLASMA_RIFLE_REFIRE			0.12f
#define RAILGUN_REFIRE				20.0f

#define PLASMA_RIFLE_DMG			14.0f
#define RAILGUN_DMG					130.0f
#define RAILGUN_BEAM_WIDTH			15.0f

class CWeaponPlasmaRifle : public CHL2MPMachineGun {

public:
	DECLARE_CLASS( CWeaponPlasmaRifle, CHL2MPMachineGun );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponPlasmaRifle();

	void			FireRifle(void);
	void			FireRailgun(void);
	virtual void	ItemPostFrame(void);
	void			Drop(const Vector &vecVelocity);
	virtual void	Precache(void);

	int				GetMinBurst( void ) { return 2; }
	int				GetMaxBurst( void ) { return 5; }
	float			GetFireRate( void ) { return PLASMA_RIFLE_REFIRE; }
	const char		*GetTracerType( void ) { return "AR2Tracer"; }

	CWeaponPlasmaRifle( const CWeaponPlasmaRifle & );

private:

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
};

#endif // HL2MP_WEAPON_PLASMA_RIFLE_H

