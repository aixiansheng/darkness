//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		PlasmaCanon - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp/weapon_plasma_canon.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
	#include "dlight.h"
#else
	#include "hl2mp_player.h"
	#include "ai_basenpc.h"
	#include "explode.h"
	#include "particle_parse.h"
#endif

#include "tier0/memdbgon.h"

#define	PLASMA_CANON_REFIRE			0.6f
#define PLASMA_CANON_STAGGER		0.2f
#define PLASMA_BOLT_DMG				35.0f
#define PLASMA_BOLT_DURATION		4.0f

#define PLASMA_BOLT_RADIUS			85.0f
#define PLASMA_HIT_SOUND			"Weapon_Mortar.Impact"

#define MECH_CANON_SPRITE			"plasma_burst"
#define MECH_PLASMA_POP				"plasma_pop"
#define MECH_PLASMA_SOUND			"TurretMissile.Loop"


#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS( plasma_bolt, CPlasmaBolt );
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPlasmaCanon, DT_WeaponPlasmaCanon )

BEGIN_NETWORK_TABLE( CWeaponPlasmaCanon, DT_WeaponPlasmaCanon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponPlasmaCanon )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_plasma_canon, CWeaponPlasmaCanon );
PRECACHE_WEAPON_REGISTER( weapon_plasma_canon );

acttable_t	CWeaponPlasmaCanon::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponPlasmaCanon);

#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponPlasmaCanon )
END_DATADESC()
#endif


CWeaponPlasmaCanon::CWeaponPlasmaCanon( void ) {
	invisible = true;
}

float CWeaponPlasmaCanon::GetDamageForActivity( Activity hitActivity ) {
	return 0.0f;
}

bool CWeaponPlasmaCanon::Deploy(void) {
	invisible = true;
	return BaseClass::Deploy();
}

void CWeaponPlasmaCanon::Drop( const Vector &vecVelocity ) {
}

float CWeaponPlasmaCanon::GetRange(void) {
	return	0.0f;	
}

float CWeaponPlasmaCanon::GetFireRate(void) {
	return	PLASMA_CANON_REFIRE;	
}

void CWeaponPlasmaCanon::ItemPostFrame( void ) {
	CBasePlayer *owner;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	if (owner->m_nButtons & IN_ATTACK) {
		if (m_flNextPrimaryAttack < gpGlobals->curtime && BaseClass::HasAnyAmmo()) {
			m_flNextPrimaryAttack = gpGlobals->curtime + PLASMA_CANON_REFIRE;
			FireCanon(true);

			// set secondary canon fire on each primary shot
			m_flNextSecondaryAttack = gpGlobals->curtime + PLASMA_CANON_STAGGER;
		}

		if (m_flNextSecondaryAttack < gpGlobals->curtime && BaseClass::HasAnyAmmo()) {
			m_flNextSecondaryAttack = gpGlobals->curtime + PLASMA_CANON_REFIRE;
			FireCanon(false);
		}
		return;
	}

	WeaponIdle();
}

//
// Fire one bolt from either the left or right canon
//
void CWeaponPlasmaCanon::FireCanon(bool left) {
	CBasePlayer *owner;

	WeaponSound(SPECIAL1);

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

#ifndef CLIENT_DLL
	
	CPlasmaBolt *bolt;

	QAngle angles;
	Vector vecForward;
	Vector origin;
	Vector fwd, right, up;
	Vector muzzle;
	Vector rightNormal;

	owner->EyeVectors(&fwd, &right, &up);
	muzzle = owner->Weapon_ShootPosition() + fwd * 12.0f;
	rightNormal = right.Normalized();

	if (left) {
		muzzle -= right * 15.0f;
	} else {
		muzzle += right * 15.0f;
	}

	VectorAngles(fwd, angles);

	bolt = CPlasmaBolt::Create(muzzle, angles, owner->edict());
	bolt->m_hOwner = this;

	owner->RemoveAmmo(1, m_iPrimaryAmmoType);

#endif

	//SendWeaponAnim( ACT_VM_HITCENTER );
	owner->SetAnimation( PLAYER_ATTACK1 );
	//ToHL2MPPlayer(owner)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
}



#ifndef CLIENT_DLL


///////////////////////////////////////////////////////////////
// Plasma bolt (plasma canon fire)
///////////////////////////////////////////////////////////////

BEGIN_DATADESC( CPlasmaBolt )

	DEFINE_FIELD( m_hOwner,					FIELD_EHANDLE ),
	DEFINE_FIELD( m_flDamage,				FIELD_FLOAT ),
	
	DEFINE_FUNCTION( BoltTouch ),
	DEFINE_FUNCTION( BurnOutThink ),

END_DATADESC()

CPlasmaBolt::CPlasmaBolt() {
}

CPlasmaBolt::~CPlasmaBolt() {
}

void CPlasmaBolt::Precache(void) {
	PrecacheModel( "models/weapons/w_missile.mdl" );
	PrecacheModel( "models/weapons/w_missile_launch.mdl" );
	PrecacheModel( "models/weapons/w_missile_closed.mdl" );
	PrecacheScriptSound(PLASMA_HIT_SOUND);
	PrecacheScriptSound(MECH_PLASMA_SOUND);
	PrecacheParticleSystem(MECH_CANON_SPRITE);
	PrecacheParticleSystem(MECH_PLASMA_POP);
}

void CPlasmaBolt::Spawn(void) {
	Precache();

	SetSolid( SOLID_BBOX );
	SetModel("models/weapons/w_missile_launch.mdl");
	UTIL_SetSize( this, -Vector(4,4,4), Vector(4,4,4) );

	SetTouch( &CPlasmaBolt::BoltTouch );
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	SetCollisionGroup(COLLISION_GROUP_PLAYER);
	
	SetThink( &CPlasmaBolt::BurnOutThink );
	SetNextThink( gpGlobals->curtime + PLASMA_BOLT_DURATION );

	m_takedamage = DAMAGE_NO;
	m_bloodColor = DONT_BLEED;

	AddEffects(EF_BRIGHTLIGHT);
}

void CPlasmaBolt::BurnOutThink(void) {
	SetThink(NULL);
	UTIL_Remove(this);
}

unsigned int CPlasmaBolt::PhysicsSolidMaskForEntity( void ) const { 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

void CPlasmaBolt::BoltTouch( CBaseEntity *pOther ) {

	//
	// plasma bolts under water--nah!
	//
	//if (pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) && 
	//	pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON )
	//	return;
	//

	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS)  && 
		pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON) {
		// triggers for doors fall into this category...
		return;
	}

	if (GetOwnerEntity() && GetOwnerEntity()->edict() == pOther->edict())
		return;

	Vector forward;

	GetVectors( &forward, NULL, NULL );

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	SetSolid(SOLID_NONE);

	CTakeDamageInfo info(m_hOwner, GetOwnerEntity(), GetDamage(), DMG_PLASMA | DMG_ALWAYSGIB);
	
	if ( tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY) ) {
		pOther->DispatchTraceAttack(info, forward, &tr);
		ApplyMultiDamage();
	}

	RadiusDamage(info, GetAbsOrigin(), PLASMA_BOLT_RADIUS, CLASS_NONE, GetOwnerEntity());
	EmitSound(PLASMA_HIT_SOUND);

	CDisablePredictionFiltering foo;
	DispatchParticleEffect(MECH_PLASMA_POP, GetAbsOrigin(), GetAbsAngles(), NULL);

	StopSound(MECH_PLASMA_SOUND);
	StopParticleEffects(this);
	UTIL_Remove( this );
}

CPlasmaBolt *CPlasmaBolt::Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner = NULL ) {
	CPlasmaBolt *pFlame;
	Vector vecForward;

	AngleVectors( vecAngles, &vecForward );

	pFlame = (CPlasmaBolt *) CBaseEntity::Create( "plasma_bolt", vecOrigin, vecAngles, CBaseEntity::Instance( pentOwner ) );
	pFlame->SetOwnerEntity( Instance( pentOwner ) );
	pFlame->Spawn();
	pFlame->AddEffects( EF_NOSHADOW );
	pFlame->SetDamage(PLASMA_BOLT_DMG);
	pFlame->SetAbsVelocity( vecForward * 1200);
	pFlame->CreateSprite();

	return pFlame;
}

void CPlasmaBolt::CreateSprite(void) {
	CDisablePredictionFiltering foo;
	DispatchParticleEffect(MECH_CANON_SPRITE, PATTACH_ABSORIGIN_FOLLOW, this);
	DispatchParticleEffect(MECH_PLASMA_POP, PATTACH_ABSORIGIN_FOLLOW, this);
	SetRenderMode(kRenderNone);

	EmitSound(MECH_PLASMA_SOUND);
}

#endif

