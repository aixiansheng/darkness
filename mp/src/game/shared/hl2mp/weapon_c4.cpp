//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "weapon_c4.h"
#include "tier0/memdbgon.h"

#ifndef CLIENT_DLL
	#include "util.h"
#endif

#define GRENADE_RADIUS				4.0f
#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

acttable_t	CWeaponC4::m_acttable[] = {
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_GRENADE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_GRENADE,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_GRENADE,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_GRENADE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_GRENADE,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_GRENADE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponC4);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponC4, DT_WeaponC4 )

BEGIN_NETWORK_TABLE( CWeaponC4, DT_WeaponC4 )
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
BEGIN_PREDICTION_DATA( CWeaponC4 )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_AttackPaused, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponC4 )
	DEFINE_THINKFUNC(PrimeThink),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( weapon_c4, CWeaponC4 );
PRECACHE_WEAPON_REGISTER(weapon_c4);

CWeaponC4::CWeaponC4( void ) : CBaseHL2MPCombatWeapon() {
	primed = false;
	priming = false;
	m_bRedraw = false;
}

void CWeaponC4::Precache( void ) {
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "grenade_c4" );
#endif

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );
}

#ifndef CLIENT_DLL
void CWeaponC4::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator ) {
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	bool fThrewGrenade = false;

	switch( pEvent->event ) {
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
	if (fThrewGrenade) {	
		m_flNextPrimaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!
	}
}

#endif

bool CWeaponC4::Deploy( void ) {
	primed = false;
	priming = false;
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Deploy();
}

bool CWeaponC4::Holster( CBaseCombatWeapon *pSwitchingTo ) {
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponC4::Reload( void ) {
	primed = false;
	priming = false;

	if ( !HasPrimaryAmmo() )
		return false;

	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) ) {
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

//
// use secondary attack to arm C4
//
void CWeaponC4::SecondaryAttack( void ) {
	CBaseCombatCharacter *pOwner;
	CBasePlayer *pPlayer;

	if (priming == true || primed == true)
		return;

	if ( m_bRedraw )
		return;

	if ( !HasPrimaryAmmo() )
		return;

	pOwner  = GetOwner();
	if ( pOwner == NULL )
		return;

	pPlayer = ToBasePlayer( pOwner );
	if ( pPlayer == NULL )
		return;

	priming = true;
	SetThink(&CWeaponC4::PrimeThink);
	SetNextThink(gpGlobals->curtime + 4.0f);
	WeaponSound(SPECIAL1);

	m_flNextSecondaryAttack	= gpGlobals->curtime + 5.0f;
}

void CWeaponC4::PrimeThink(void) {
	primed = true;
	priming = false;

#ifndef CLIENT_DLL
	CHL2MP_Player *p;
	
	p = ToHL2MPPlayer(GetOwner());
	if (!p)
		return;

	p->DropC4OnHit(true);

	UTIL_SayText("Caution: C4 armed and dangerous!\n", p);

#endif
	
}

void CWeaponC4::PrimaryAttack( void ) {
	
	CBaseCombatCharacter *pOwner;
	CBasePlayer *pPlayer;

	if ( priming == true || primed == false)
		return;

	if ( m_bRedraw )
		return;

	pOwner  = GetOwner();
	if ( pOwner == NULL ) { 
		return;
	}

	pPlayer = ToBasePlayer( pOwner );
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

void CWeaponC4::DecrementAmmo( CBaseCombatCharacter *pOwner ) {
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

void CWeaponC4::ItemPostFrame( void ) {
	if ( m_fDrawbackFinished ) {
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		if (pOwner) {
			switch( m_AttackPaused ) {
			case GRENADE_PAUSED_PRIMARY:
				if( !(pOwner->m_nButtons & IN_ATTACK) ) {
					SendWeaponAnim( ACT_VM_THROW );
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

	if ( m_bRedraw ) {
		if ( IsViewModelSequenceFinished() ) {
			Reload();
		}
	}
}

void CWeaponC4::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc ) {
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() ) {
		vecSrc = tr.endpos;
	}
}


void CWeaponC4::ThrowGrenade( CBasePlayer *pPlayer ) {
#ifndef CLIENT_DLL
	CHL2MP_Player *hl2mp_player;
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 20.0f + vRight * 8.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 200;
	CBaseGrenade *pGrenade = C4_Create( vecSrc, vec3_angle, vecThrow, pPlayer, C4_TIMER);

	if ( pGrenade ) {
		if ( pPlayer && pPlayer->m_lifeState != LIFE_ALIVE ) {
			pPlayer->GetVelocity( &vecThrow, NULL );
		}
		
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
	}

	hl2mp_player = ToHL2MPPlayer(pPlayer);
	hl2mp_player->DropC4OnHit(false);

#endif

	m_bRedraw = true;

	WeaponSound( SINGLE );
	
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}
