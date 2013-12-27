#ifndef HL2MP_WEAPON_BREEDER_H
#define HL2MP_WEAPON_BREEDER_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#ifndef CLIENT_DLL
	#include "util.h"
#endif


enum destroy_state {
	STATE_OFF,
	STATE_CHARGING
};

#ifdef CLIENT_DLL
#define CWeaponBreeder C_WeaponBreeder
#endif

class CWeaponBreeder : public CBaseHL2MPBludgeonWeapon {
public:
	DECLARE_CLASS( CWeaponBreeder, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponBreeder();

	float		GetRange( void );
	float		GetFireRate( void );

	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack(void);
	void		PrimaryAttack(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );
	virtual void ItemPostFrame( void );
	virtual bool ShouldDraw(void) { return false; }
	void MakeItem(int idx);

	CWeaponBreeder( const CWeaponBreeder & );


#ifndef CLIENT_DLL
	void ItemStatusUpdate(CBasePlayer *player, int health, int armor);
	void WarnPlayer(void);
	virtual void DisplayUsageHudHint(void);
#endif

private:
	float m_flNextItemStatus;

	enum destroy_state destroy_state;
	float m_flDestroyTime;
	float m_flNextItemCreation;

	CNetworkVar(float, m_flNextDestroyBeep);

};


#endif // HL2MP_WEAPON_BREEDER_H

