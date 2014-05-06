//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//

#ifndef	WEAPONC4_H
#define	WEAPONC4_H

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "grenade_c4.h"
#endif

#include "weapon_ar2.h"
#include "effect_dispatch_data.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"


#define C4_TIMER					15.0f

#ifdef CLIENT_DLL
#define CWeaponC4 C_WeaponC4
#endif

class CWeaponC4: public CBaseHL2MPCombatWeapon {

	DECLARE_CLASS( CWeaponC4, CBaseHL2MPCombatWeapon );

public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CWeaponC4();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );

	virtual void Drop(const Vector &vecVelocity) {}
	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	bool	Reload( void );
	
	void	PrimeThink(void);

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	void	ThrowGrenade( CBasePlayer *pPlayer );
	bool	IsPrimed( bool ) { return ( m_AttackPaused != 0 );	}
	
private:
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	CNetworkVar( bool,	m_bRedraw );
	CNetworkVar( int,	m_AttackPaused );
	CNetworkVar( bool,	m_fDrawbackFinished );

	bool priming;
	bool primed;

	CWeaponC4( const CWeaponC4 & );
	DECLARE_ACTTABLE();
};

#endif	//WEAPONC4_H
