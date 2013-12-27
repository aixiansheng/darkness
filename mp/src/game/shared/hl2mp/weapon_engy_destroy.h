#ifndef HL2MP_WEAPON_ENGY_DESTROY_H
#define HL2MP_WEAPON_ENGY_DESTROY_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL
#define CWeaponEngyDestroy C_WeaponEngyDestroy
#include "c_team.h"
#endif

class CWeaponEngyDestroy : public CBaseHL2MPBludgeonWeapon {
public:
	DECLARE_CLASS( CWeaponEngyDestroy, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponEngyDestroy();

	float		GetRange( void );
	float		GetFireRate( void );

	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack(void);
	void		PrimaryAttack(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );

	virtual void ItemPostFrame( void );

	CWeaponEngyDestroy( const CWeaponEngyDestroy & );


#ifndef CLIENT_DLL
	void ItemStatusUpdate(CBasePlayer *player, int health, int armor);
	virtual void DisplayUsageHudHint(void);
#endif

private:
	float m_flNextItemStatus;
};


#endif // HL2MP_WEAPON_ENGY_DESTROY_H

