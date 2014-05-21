#ifndef HL2MP_WEAPON_BREEDER_H
#define HL2MP_WEAPON_BREEDER_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#ifndef CLIENT_DLL
	#include "util.h"
#endif


enum hatch_state {
	STATE_OFF,
	STATE_WAITING_EGG,
	STATE_WAITING_NORMAL,
	STATE_CHARGING_DESTROY,
	STATE_DIGESTING
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

	float		GetDamageForActivity(Activity hitActivity);

	enum hatch_state BreederSecondaryAttack(void);
	enum hatch_state BreederPrimaryAttack(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );
	virtual void ItemPostFrame( void );
	virtual bool ShouldDraw(void) { return false; }
	void MakeItem(int idx);

	CWeaponBreeder( const CWeaponBreeder & );

	void StartDestroyCharge(void);

	void HandleItemUpdate(void);

#ifndef CLIENT_DLL

	virtual void Precache(void);
	void ItemStatusUpdate(CBasePlayer *player, int health, int armor, int maxhealth);
	
#endif

private:

	float m_flNextItemStatus;

	float m_flNextItemCreation;

	CNetworkVar(float, m_flNextFXTime);
	CNetworkVar(float, m_flNextHatchTime);
	CNetworkVar(enum hatch_state, hatch_state);
};


#endif // HL2MP_WEAPON_BREEDER_H

