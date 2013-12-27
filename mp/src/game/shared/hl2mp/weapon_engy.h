#ifndef HL2MP_WEAPON_ENGY_H
#define HL2MP_WEAPON_ENGY_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL
#define CWeaponEngy C_WeaponEngy
#include "c_team.h"
#endif

class CWeaponEngy : public CBaseHL2MPBludgeonWeapon {
public:
	DECLARE_CLASS( CWeaponEngy, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponEngy();

	float		GetRange( void );
	float		GetFireRate( void );

	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack(void);
	void		PrimaryAttack(void);
	void		MakeItem(int idx);

	void		Drop(const Vector &vecVelocity);
	bool		Deploy(void);

	virtual void ItemPostFrame(void);
	

	CWeaponEngy( const CWeaponEngy & );


#ifndef CLIENT_DLL
	virtual void Precache(void);
	void ItemStatusUpdate(CBasePlayer *player, int health, int armor);
	virtual void DisplayUsageHudHint(void);

	unsigned int repaired_total;

#endif

private:
	float m_flNextItemStatus;
	float m_flNextItemCreation;
};


#endif // HL2MP_WEAPON_ENGY_H

