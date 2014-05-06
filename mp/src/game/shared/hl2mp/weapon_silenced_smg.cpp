//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "grenade_ar2.h"
	#include "hl2mp_player.h"
	#include "basegrenade_shared.h"
#endif

#include "weapon_hl2mpbase.h"
#include "weapon_hl2mpbase_machinegun.h"

#ifdef CLIENT_DLL
#define CWeaponSilencedSMG1 C_WeaponSilencedSMG1
#endif

#include "tier0/memdbgon.h"

class CWeaponSilencedSMG1 : public CHL2MPMachineGun {
public:
	DECLARE_CLASS( CWeaponSilencedSMG1, CHL2MPMachineGun );

	CWeaponSilencedSMG1();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void	Precache( void );
	void	AddViewKick( void );

	int		GetMinBurst() { return 3; }
	int		GetMaxBurst() { return 3; }
	virtual void Drop(const Vector &vecVelocity);

	virtual void Equip( CBaseCombatCharacter *pOwner );
	bool	Reload( void );

	float	GetFireRate( void ) { return 0.075f; }	// 13.3hz
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void ) {
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	
private:
	CWeaponSilencedSMG1( const CWeaponSilencedSMG1 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSilencedSMG1, DT_WeaponSilencedSMG1 )

BEGIN_NETWORK_TABLE( CWeaponSilencedSMG1, DT_WeaponSilencedSMG1 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSilencedSMG1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_silenced_smg1, CWeaponSilencedSMG1 );
PRECACHE_WEAPON_REGISTER(weapon_silenced_smg1);

acttable_t	CWeaponSilencedSMG1::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_SMG1,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_SMG1,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_SMG1,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_SMG1,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_SMG1,					false },
};

IMPLEMENT_ACTTABLE(CWeaponSilencedSMG1);

//=========================================================
CWeaponSilencedSMG1::CWeaponSilencedSMG1( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSilencedSMG1::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther("grenade_ar2");
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponSilencedSMG1::Equip( CBaseCombatCharacter *pOwner )
{
	m_fMaxRange1 = 1400;

	BaseClass::Equip( pOwner );
}

Activity CWeaponSilencedSMG1::GetPrimaryAttackActivity( void )
{
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

void CWeaponSilencedSMG1::Drop(const Vector &vecVelocity) {
}

bool CWeaponSilencedSMG1::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );
		//ToHL2MPPlayer(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );

	}

	return fRet;
}

void CWeaponSilencedSMG1::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	0.5f	//Degrees
	#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponSilencedSMG1::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
