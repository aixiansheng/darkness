//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		StalkerSlash - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp/weapon_stalker_slash.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ents/spike.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "ai_basenpc.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	STALKER_SLASH_RANGE		125.0f
#define	STALKER_SLASH_REFIRE	0.55f
#define STALKER_SLASH_DAMAGE	120.0f

#define STALKER_SPIKE_RELOAD_TIME 1.5f
#define STALKER_SPIKE_DMG 40.0f
#define STALKER_SPIKE_SPEED 800.0f

#define STALKER_SPIKE_RECHARGE_INT 10.0f

//-----------------------------------------------------------------------------
// CWeaponStalkerSlash
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponStalkerSlash, DT_WeaponStalkerSlash )

BEGIN_NETWORK_TABLE( CWeaponStalkerSlash, DT_WeaponStalkerSlash )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponStalkerSlash )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_stalker_slash, CWeaponStalkerSlash );
PRECACHE_WEAPON_REGISTER( weapon_stalker_slash );

acttable_t	CWeaponStalkerSlash::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponStalkerSlash);

CWeaponStalkerSlash::CWeaponStalkerSlash( void  ) {
}

float CWeaponStalkerSlash::GetDamageForActivity( Activity hitActivity ) {
	return STALKER_SLASH_DAMAGE;
}

bool CWeaponStalkerSlash::Deploy(void) {
	m_flNextSecondaryAttack = gpGlobals->curtime + STALKER_SPIKE_RELOAD_TIME;
	return BaseClass::Deploy();
}

void CWeaponStalkerSlash::AddViewKick(void) {
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = SharedRandomFloat( "crowbarpax", 1.0f, 2.0f );
	punchAng.y = SharedRandomFloat( "crowbarpay", -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}

#ifndef CLIENT_DLL

void CWeaponStalkerSlash::RechargeThink(void) {
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(GetOwner());
	if (!p)
		return;

	p->GiveAmmo(1, m_iSecondaryAmmoType, true);

	if (p->GetAmmoCount(m_iSecondaryAmmoType) < GetAmmoDef()->MaxCarry(m_iSecondaryAmmoType)) {
		SetNextThink(gpGlobals->curtime + STALKER_SPIKE_RECHARGE_INT);
	} else {
		SetThink(NULL);
	}
}

void CWeaponStalkerSlash::ResetRecharge(void) {
	SetThink(&CWeaponStalkerSlash::RechargeThink);
	SetNextThink(gpGlobals->curtime + STALKER_SPIKE_RECHARGE_INT);
}

void CWeaponStalkerSlash::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator ) {
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition(), vecEnd, 
		Vector(-16,-16,-16), Vector(36,36,36), GetDamageForActivity( GetActivity() ), DMG_CLUB, 0.75 );
	
	// did I hit someone?
	if ( pHurt )
	{
		// play sound
		WeaponSound( MELEE_HIT );

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit );
		ImpactEffect( traceHit );
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}
}


void CWeaponStalkerSlash::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator ) {
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

ConVar sk_stalker_slash_lead_time( "sk_stalker_slash_lead_time", "0.9" );

int CWeaponStalkerSlash::WeaponMeleeAttack1Condition( float flDot, float flDist ) {
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_stalker_slash_lead_time.GetFloat();
	dt += SharedRandomFloat( "crowbarmelee1", -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

#endif


void CWeaponStalkerSlash::ItemPostFrame( void ) {
	CBasePlayer *owner;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	if ((owner->m_nButtons & IN_ATTACK) && 
		m_flNextPrimaryAttack < gpGlobals->curtime) {
		BaseClass::PrimaryAttack();
		return;
	}
	
	if ((owner->m_nButtons & IN_ATTACK2) && 
		m_flNextSecondaryAttack < gpGlobals->curtime &&
		owner->GetAmmoCount(m_iSecondaryAmmoType) >= 2) {
		SecondaryAttack();
		return;
	}


	WeaponIdle();

}

void CWeaponStalkerSlash::PrimaryAttack(void) {
	m_flNextPrimaryAttack = gpGlobals->curtime + STALKER_SLASH_REFIRE;
	BaseClass::PrimaryAttack();
}

void CWeaponStalkerSlash::SecondaryAttack(void) {
	m_flNextSecondaryAttack = gpGlobals->curtime + STALKER_SPIKE_RELOAD_TIME;

	WeaponSound(SPECIAL1);

#ifndef CLIENT_DLL
	CBasePlayer *owner;
	Vector src_left;
	Vector src_right;
	Vector fwd, right, up;
	QAngle angles;
	CSpike *left_spike;
	CSpike *right_spike;
	
	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	owner->EyeVectors(&fwd, &right, &up);
	src_left = owner->Weapon_ShootPosition() + fwd * 10.0f + (right * -7.0f);
	src_right = owner->Weapon_ShootPosition() + fwd * 10.0f + (right * 7.0f);

	VectorAngles(fwd, angles);
	(void)VectorNormalize(fwd);

	left_spike = Spike_Create(src_left, angles, fwd * STALKER_SPIKE_SPEED, owner, STALKER_SPIKE_DMG, true);
	right_spike = Spike_Create(src_right, angles, fwd * STALKER_SPIKE_SPEED, owner, STALKER_SPIKE_DMG, true);
	(void)left_spike;
	(void)right_spike;

	owner->RemoveAmmo(2, m_iSecondaryAmmoType);

	ResetRecharge();

#endif
}

void CWeaponStalkerSlash::Drop( const Vector &vecVelocity )
{
}

float CWeaponStalkerSlash::GetRange( void )
{
	return	STALKER_SLASH_RANGE;	
}

float CWeaponStalkerSlash::GetFireRate( void )
{
	return	STALKER_SLASH_REFIRE;	
}


