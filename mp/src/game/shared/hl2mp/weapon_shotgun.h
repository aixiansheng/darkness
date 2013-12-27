#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "explode.h"
	#include "te_effect_dispatch.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponShotgun C_WeaponShotgun
#endif

#define XP_SHELL_DMG	100.0f
#define XP_SHELL_RAD	125.0f


class CWeaponShotgun : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponShotgun, CBaseHL2MPCombatWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

private:
	CNetworkVar( bool,	m_bNeedPump );		// When emptied completely
	CNetworkVar( bool,	m_bDelayedFire1 );	// Fire primary when finished reloading
	CNetworkVar( bool,	m_bDelayedFire2 );	// Fire secondary when finished reloading
	CNetworkVar( bool,	m_bDelayedReload );	// Reload when finished pump

	void ToggleAmmoType(void);

public:
	
	CNetworkVar(int, currentAmmoType);

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_15DEGREES;
		return cone;
	}

	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	bool StartReload( void );
	bool Reload( void );
	void FillClip( void );
	void FinishReload( void );
	void CheckHolsterReload( void );
	void Pump( void );
	void ItemHolsterFrame( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void DryFire( void );
	virtual float GetFireRate( void ) { return 0.7; };

#ifndef CLIENT_DLL
	virtual void Spawn(void);
	virtual void DisplayUsageHudHint(void);
#endif

	DECLARE_ACTTABLE();

	CWeaponShotgun(void);

private:
	CWeaponShotgun( const CWeaponShotgun & );
};