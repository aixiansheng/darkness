//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		StingerFire - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp/weapon_stinger_fire.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "ai_basenpc.h"
	#include "explode.h"
	#include "particle_parse.h"
#endif

#define STINGER_FIRE_EFFECT "stinger_fire"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	STINGER_FIRE_RANGE	150.0f
#define	STINGER_FIRE_REFIRE	0.115f
#define STINGER_SLASH_DAMAGE 80.0f
#define STINGER_SLASH_REFIRE 0.75f

#define STINGER_FLAME_DMG 15.0f
#define STINGER_FLAME_DURATION 0.4f

//-----------------------------------------------------------------------------
// CWeaponStingerFire
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponStingerFire, DT_WeaponStingerFire )

BEGIN_NETWORK_TABLE( CWeaponStingerFire, DT_WeaponStingerFire )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponStingerFire )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_stinger_fire, CWeaponStingerFire );
PRECACHE_WEAPON_REGISTER( weapon_stinger_fire );

acttable_t	CWeaponStingerFire::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponStingerFire);

#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponStingerFire )

	DEFINE_FUNCTION( RechargeThink ),

END_DATADESC()
#endif


///////////////////////////////////////////////////////////////
// Stinger weapon
///////////////////////////////////////////////////////////////


CWeaponStingerFire::CWeaponStingerFire( void ) {
}

float CWeaponStingerFire::GetDamageForActivity( Activity hitActivity ) {
	return STINGER_SLASH_DAMAGE;
}

#define FLAME_RECHARGE_TIME 0.2f
#define FLAME_RECHARGE_AMT 1
#define MAX_FLAME_COUNT 10

void CWeaponStingerFire::RechargeThink(void) {
#ifndef CLIENT_DLL
	CBasePlayer *owner;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	if (owner->GetAmmoCount(m_iPrimaryAmmoType) >= MAX_FLAME_COUNT) {
		SetThink(NULL);
		return;
	}

	owner->GiveAmmo(1, m_iPrimaryAmmoType, true);
	SetNextThink(gpGlobals->curtime + FLAME_RECHARGE_TIME);
#endif
}

bool CWeaponStingerFire::Deploy(void) {
	return BaseClass::Deploy();
}

void CWeaponStingerFire::Drop( const Vector &vecVelocity ) {
}

float CWeaponStingerFire::GetRange(void) {
	return	STINGER_FIRE_RANGE;	
}

float CWeaponStingerFire::GetFireRate(void) {
	return	STINGER_FIRE_REFIRE;	
}

void CWeaponStingerFire::ItemPostFrame( void ) {
	CBasePlayer *owner;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	if (owner->m_nButtons & IN_ATTACK) {
		SetThink(NULL);
		charging = false;

		if (m_flNextPrimaryAttack < gpGlobals->curtime && BaseClass::HasAnyAmmo()) {
			PrimaryAttack();
		}
		
		return;
	}
	
	if (owner->m_nButtons & IN_ATTACK2) {

		if (m_flNextSecondaryAttack < gpGlobals->curtime) {
			SecondaryAttack();
		}

		return;
	}

	WeaponIdle();

#ifndef CLIENT_DLL
	// recharge
	if (!charging && owner->GetAmmoCount(m_iPrimaryAmmoType) < MAX_FLAME_COUNT) {
		charging = true;
		SetThink(&CWeaponStingerFire::RechargeThink);
		SetNextThink(gpGlobals->curtime + 0.4f);
	}
#endif

}

#ifndef CLIENT_DLL
void CWeaponStingerFire::Spawn(void) {
	BaseClass::Spawn();
	SetAmmoType(GetAmmoDef()->Index("stinger_slash"));
}
#endif


//////////////////////////////////////////////
// Secondary attack = slash
//////////////////////////////////////////////
void CWeaponStingerFire::SecondaryAttack(void) {
	BaseClass::SecondaryAttack();
	m_flNextSecondaryAttack = gpGlobals->curtime + STINGER_SLASH_REFIRE;
}

//////////////////////////////////////////////
// Primary attack = fire
//////////////////////////////////////////////
void CWeaponStingerFire::PrimaryAttack(void) {
	m_flNextPrimaryAttack = gpGlobals->curtime + STINGER_FIRE_REFIRE;

	WeaponSound(SPECIAL1);

#ifndef CLIENT_DLL
	CBasePlayer *owner;

	CStingerFire *fire;
	QAngle angles;
	Vector vecForward;
	Vector origin;
	Vector fwd, right, up;
	Vector muzzle;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	owner->EyeVectors(&fwd, &right, &up);
	muzzle = owner->Weapon_ShootPosition() + fwd * 12.0f;
	muzzle.z -= 8.0f;

	VectorAngles(fwd, angles);

	fire = CStingerFire::Create(muzzle, angles, owner->edict());
	if (fire) {
		fire->m_hOwner = this;
		fire->SetDamage(50);
		owner->RemoveAmmo(1, m_iPrimaryAmmoType);
	}

#endif

}



///////////////////////////////////////////////////////////////
// Stinger fire (fireball)
///////////////////////////////////////////////////////////////


CStingerFire::CStingerFire() {
}

CStingerFire::~CStingerFire() {
}

IMPLEMENT_NETWORKCLASS_ALIASED( StingerFire, DT_StingerFire )

BEGIN_NETWORK_TABLE( CStingerFire, DT_StingerFire )
END_NETWORK_TABLE()


#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS( stinger_fireball, CStingerFire );

BEGIN_DATADESC( CStingerFire )

	DEFINE_FIELD( m_hOwner,					FIELD_EHANDLE ),
	DEFINE_FIELD( m_flMarkDeadTime,			FIELD_TIME ),
	DEFINE_FIELD( m_flDamage,				FIELD_FLOAT ),
	
	DEFINE_FUNCTION( FlameTouch ),
	DEFINE_FUNCTION( BurnThink ),

END_DATADESC()



void CStingerFire::Precache(void) {
	PrecacheModel( "models/weapons/w_missile.mdl" );
	PrecacheModel( "models/weapons/w_missile_launch.mdl" );
	PrecacheModel( "models/weapons/w_missile_closed.mdl" );
	PrecacheParticleSystem(STINGER_FIRE_EFFECT);
}

void CStingerFire::Spawn(void) {
	Precache();

	SetSolid( SOLID_BBOX );
	SetModel("models/weapons/w_missile_launch.mdl");
	UTIL_SetSize( this, -Vector(4,4,4), Vector(4,4,4) );

	SetTouch( &CStingerFire::FlameTouch );
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_BOUNCE );
	SetCollisionGroup(COLLISION_GROUP_PLAYER);
	
	SetThink( &CStingerFire::BurnThink );
	SetNextThink( gpGlobals->curtime + 0.3f );

	m_takedamage = DAMAGE_NO;
	m_bloodColor = DONT_BLEED;

	m_flMarkDeadTime = gpGlobals->curtime + STINGER_FLAME_DURATION;

	AddEffects(EF_BRIGHTLIGHT);
}

void CStingerFire::BurnThink(void) {
	if (m_flMarkDeadTime < gpGlobals->curtime) {
		UTIL_Remove(this);
		SetThink(NULL);
		return;
	}

	//scale += 1.0f;
	//flameSprite->SetScale(scale);

	SetNextThink(gpGlobals->curtime + 0.1f);
}

unsigned int CStingerFire::PhysicsSolidMaskForEntity( void ) const { 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

void CStingerFire::FlameTouch( CBaseEntity *pOther ) {
	if (pOther == NULL)
		return;

	//if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) &&
	// pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON )
	//	return;

	Vector forward;

	GetVectors( &forward, NULL, NULL );

	trace_t tr;
	UTIL_TraceLine
	(
		GetAbsOrigin(),
		GetAbsOrigin() + forward * 16,
		MASK_SHOT,
		this,
		COLLISION_GROUP_NONE,
		&tr
	);

	SetSolid(SOLID_NONE);

	if (tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY)) {
		CTakeDamageInfo info(m_hOwner, this, STINGER_FLAME_DMG, DMG_BURN);
		info.SetAmmoType(GetAmmoDef()->Index("stingerfire"));

		pOther->DispatchTraceAttack(info, forward, &tr);
		ApplyMultiDamage();
	}

	UTIL_Remove( this );
}

CStingerFire *CStingerFire::Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner = NULL ) {
	CStingerFire *pFlame;
	Vector vecForward;

	pFlame = (CStingerFire *) CBaseEntity::Create( "stinger_fireball", vecOrigin, vecAngles, CBaseEntity::Instance( pentOwner ) );
	if (pFlame) {
		pFlame->SetOwnerEntity( Instance( pentOwner ) );
		pFlame->Spawn();
		pFlame->AddEffects( EF_NOSHADOW );
	
		AngleVectors( vecAngles, &vecForward );

		pFlame->SetAbsVelocity( vecForward * 1200);

		pFlame->CreateSprite();
	}

	return pFlame;
}

void CStingerFire::CreateSprite(void) {
	CDisablePredictionFiltering foo;
	DispatchParticleEffect(STINGER_FIRE_EFFECT, PATTACH_ABSORIGIN_FOLLOW, this);
	SetRenderMode(kRenderNone);
}

#else

void CStingerFire::Simulate(void) {
	BaseClass::Simulate();

	if (IsEffectActive(EF_BRIGHTLIGHT)) {
		dlight_t *dl = effects->CL_AllocDlight(index);
		dl->origin = GetAbsOrigin();
		dl->color.r = 255;
		dl->color.g = 194;
		dl->color.b = 30;
		dl->radius = 512;
		dl->die = gpGlobals->curtime + 0.01f;
	}
}

bool CStingerFire::ShouldInterpolate(void) {
	return true;
}


#endif

