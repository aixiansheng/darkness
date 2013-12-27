//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "grenade_guardian.h"
#endif

#include "weapon_ar2.h"
#include "effect_dispatch_data.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_TIMER				1.2f 
#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f
#define GRENADE_DAMAGE_RADIUS 250.0f

#ifdef CLIENT_DLL
#define CWeaponGuardianGren C_WeaponGuardianGren
#endif

class CWeaponGuardianGren: public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponGuardianGren, CBaseHL2MPCombatWeapon );
public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponGuardianGren();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	bool	Reload( void );
	virtual bool ShouldDraw(void) { return false; }

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

	CWeaponGuardianGren( const CWeaponGuardianGren & );
	DECLARE_ACTTABLE();
};

acttable_t	CWeaponGuardianGren::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponGuardianGren);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGuardianGren, DT_WeaponGuardianGren )

BEGIN_NETWORK_TABLE( CWeaponGuardianGren, DT_WeaponGuardianGren )

#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bRedraw ) ),
	RecvPropBool( RECVINFO( m_fDrawbackFinished ) ),
	RecvPropInt( RECVINFO( m_AttackPaused ) ),
#else
	SendPropBool( SENDINFO( m_bRedraw ) ),
	SendPropBool( SENDINFO( m_fDrawbackFinished ) ),
	SendPropInt( SENDINFO( m_AttackPaused ) ),
#endif
	
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponGuardianGren )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_AttackPaused, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_guard_gren, CWeaponGuardianGren );
PRECACHE_WEAPON_REGISTER(weapon_guard_gren);

CWeaponGuardianGren::CWeaponGuardianGren( void ) : CBaseHL2MPCombatWeapon() {
	m_bRedraw = false;
	invisible = true;
}

void CWeaponGuardianGren::Precache( void ) {
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "grenade_guardian" );
#endif

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );
}

#ifndef CLIENT_DLL
void CWeaponGuardianGren::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator ) {
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	bool fThrewGrenade = false;

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_fDrawbackFinished = true;
			break;

		case EVENT_WEAPON_THROW:
		case EVENT_WEAPON_THROW2:
		case EVENT_WEAPON_THROW3:
			ThrowGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}

#define RETHROW_DELAY	0.5
	if( fThrewGrenade ) {	
		m_flNextPrimaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!
	}
}

#endif

bool CWeaponGuardianGren::Deploy( void ) {
	bool ret;

	m_bRedraw = false;
	m_fDrawbackFinished = false;

	ret = BaseClass::Deploy();

	invisible = true;

	return ret;
}

bool CWeaponGuardianGren::Holster( CBaseCombatWeapon *pSwitchingTo ) {
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponGuardianGren::Reload( void ) {
	if ( !HasPrimaryAmmo() )
		return false;

	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}

void CWeaponGuardianGren::SecondaryAttack( void ) {
	if ( m_bRedraw )
		return;

	if ( !HasPrimaryAmmo() )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pOwner );
	
	if ( pPlayer == NULL )
		return;

	// Note that this is a secondary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_SECONDARY;
	SendWeaponAnim( ACT_VM_PULLBACK_LOW );

	// Don't let weapon idle interfere in the middle of a throw!
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextSecondaryAttack	= FLT_MAX;

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() )
	{
		pPlayer->SwitchToNextBestWeapon( this );
	}
}

void CWeaponGuardianGren::PrimaryAttack( void ) {
	if ( m_bRedraw )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL ) { 
		return;
	}

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );;

	if ( !pPlayer )
		return;

	// Note that this is a primary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_PRIMARY;
	SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
	
	// Put both of these off indefinitely. We do not know how long
	// the player will hold the grenade.
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() ) {
		pPlayer->SwitchToNextBestWeapon( this );
	}
}

void CWeaponGuardianGren::DecrementAmmo( CBaseCombatCharacter *pOwner ) {
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

void CWeaponGuardianGren::ItemPostFrame( void ) {
	if( m_fDrawbackFinished )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		if (pOwner)
		{
			switch( m_AttackPaused )
			{
			case GRENADE_PAUSED_PRIMARY:
				if( !(pOwner->m_nButtons & IN_ATTACK) )
				{
					SendWeaponAnim( ACT_VM_THROW );
					//ToHL2MPPlayer(pOwner)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

					//Tony; fire the sequence
					m_fDrawbackFinished = false;
				}
				break;

			case GRENADE_PAUSED_SECONDARY:
				if( !(pOwner->m_nButtons & IN_ATTACK2) )
				{
					//See if we're ducking
					if ( pOwner->m_nButtons & IN_DUCK )
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_SECONDARYATTACK );
					}
					else
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_HAULBACK );
					}
					//Tony; the grenade really should have a secondary anim. but it doesn't on the player.
					//ToHL2MPPlayer(pOwner)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

					m_fDrawbackFinished = false;
				}
				break;

			default:
				break;
			}
		}
	}

	BaseClass::ItemPostFrame();

	if ( m_bRedraw )
	{
		if ( IsViewModelSequenceFinished() )
		{
			Reload();
		}
	}
}

void CWeaponGuardianGren::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc ) {
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}


void CWeaponGuardianGren::ThrowGrenade( CBasePlayer *pPlayer ) {
#ifndef CLIENT_DLL
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 20.0f + vRight * 8.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 800;
	CBaseGrenade *pGrenade = GuardianGren_Create( vecSrc, vec3_angle, vecThrow, pPlayer, GRENADE_TIMER, true);

	if ( pGrenade ) {
		if ( pPlayer && pPlayer->m_lifeState != LIFE_ALIVE ) {
			pPlayer->GetVelocity( &vecThrow, NULL );
		}
		
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( GRENADE_DAMAGE_RADIUS );
	}
#endif

	m_bRedraw = true;

	WeaponSound( SINGLE );
	
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}
