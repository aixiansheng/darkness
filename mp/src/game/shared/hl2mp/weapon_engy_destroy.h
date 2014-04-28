#ifndef HL2MP_WEAPON_ENGY_DESTROY_H
#define HL2MP_WEAPON_ENGY_DESTROY_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"

enum destroy_gun_state {
	STATE_OFF,
	STATE_CHARGING,
	STATE_CHARGED,
	STATE_DRAINING
};

typedef enum destroy_gun_state destroy_gun_state_t;

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
	void		PrimaryAttack(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );

	virtual void ItemPostFrame( void );

	CWeaponEngyDestroy( const CWeaponEngyDestroy & );

	void ChargeThink(void);
	void DrainThink(void);


#ifndef CLIENT_DLL

	virtual void Spawn(void);
	void ItemStatusUpdate(CBasePlayer *player, int health, int armor);

	DECLARE_DATADESC();

#endif

private:
	float m_flNextItemStatus;
	
	CNetworkVar(float, nextWarnBeep);
	CNetworkVar(int, warnBeepsLeft);
	CNetworkVar(destroy_gun_state_t, gun_state);
};


#endif // HL2MP_WEAPON_ENGY_DESTROY_H

