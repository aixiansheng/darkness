//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		DroneSlash - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp/weapon_drone_slash.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "particle_parse.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "ai_basenpc.h"
	#include "Sprite.h"
	#include "te_effect_dispatch.h"
	#include "IEffects.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DRONE_SPIT_SOUND	"Drone.Spit"

#define	DRONESLASH_RANGE		85.0f
#define	DRONESLASH_REFIRE		0.6f
#define DRONESLASH_DAMAGE		60.0f

#define SPIT_MODEL				"models/Gibs/Antlion_gib_small_1.mdl"

#define SPIT_VELOCITY			420
#define SPIT_SKIN_NORMAL		0
#define SPIT_SKIN_GLOW			1
#define SPIT_DMG				0

#define MAX_SPIT_COUNT			7
#define SPIT_RECHARGE_TIME		0.2f
#define SPIT_TIME				0.30f


#ifndef CLIENT_DLL

//
// Drone spit projectile (derived from CCrossbowBolt)
//
class CDroneSpit : public CBaseCombatCharacter
{
	DECLARE_CLASS( CDroneSpit, CBaseCombatCharacter );

public:
	CDroneSpit() { };
	~CDroneSpit();

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void SpitTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CDroneSpit *SpitCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL );

protected:

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

LINK_ENTITY_TO_CLASS( drone_spit, CDroneSpit );

BEGIN_DATADESC( CDroneSpit )
	DEFINE_FUNCTION( SpitTouch ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CDroneSpit, DT_CDroneSpit )
END_SEND_TABLE()


//
// Class method to spawn drone spit
//
CDroneSpit *CDroneSpit::SpitCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner ) {
	CDroneSpit *pSpit = (CDroneSpit *)CreateEntityByName( "drone_spit" );

	UTIL_SetOrigin( pSpit, vecOrigin );

	pSpit->SetAbsAngles( angAngles );
	pSpit->Spawn();
	pSpit->SetOwnerEntity( pentOwner );

	// parent entity velocity?

	return pSpit;
}


CDroneSpit::~CDroneSpit( void ) {
}

bool CDroneSpit::CreateVPhysics( void ) {
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );
	return true;
}

unsigned int CDroneSpit::PhysicsSolidMaskForEntity() const {
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

void CDroneSpit::Spawn( void ) {
	Precache( );

	SetModel( SPIT_MODEL );
	
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector(5,5,5), Vector(5,5,5) );

	SetGravity( 0.85f );
	SetTouch( &CDroneSpit::SpitTouch );
}


void CDroneSpit::Precache( void ) {
	PrecacheModel( SPIT_MODEL );
}

void CDroneSpit::SpitTouch( CBaseEntity *pOther ) {
	
	if (GetOwnerEntity() && pOther->entindex() == GetOwnerEntity()->entindex())
		return;

	if ( pOther->m_takedamage != DAMAGE_NO ) {
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector vForward;
		Vector	vecNormalizedVel = GetAbsVelocity();
		VectorNormalize(vecNormalizedVel);

		ClearMultiDamage();

		if (GetOwnerEntity()) {
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), SPIT_DMG, DMG_PARALYZE );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}

		ApplyMultiDamage();
	}

	SetAbsVelocity( Vector( 0, 0, 0 ) );
	SetTouch( NULL );
	UTIL_Remove( this );
}

#else

//
// Client-side implementation of Drone spit
//
class C_DroneSpit : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_DroneSpit, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
public:
	
	C_DroneSpit( void );

	virtual void	ClientThink( void );
	virtual void	OnDataChanged( DataUpdateType_t updateType );

private:

	C_DroneSpit( const C_DroneSpit & );

	Vector	m_vecLastOrigin;
	bool	m_bUpdated;
};

IMPLEMENT_CLIENTCLASS_DT( C_DroneSpit, DT_CDroneSpit, CDroneSpit )
END_RECV_TABLE()

C_DroneSpit::C_DroneSpit( void ) {}

void C_DroneSpit::OnDataChanged( DataUpdateType_t updateType ) {
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED ) {
		m_bUpdated = false;
		m_vecLastOrigin = GetAbsOrigin();
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

void C_DroneSpit::ClientThink( void ) {
	m_bUpdated = false;
}

#endif


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDroneSlash, DT_WeaponDroneSlash )

BEGIN_NETWORK_TABLE( CWeaponDroneSlash, DT_WeaponDroneSlash )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponDroneSlash )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_drone_slash, CWeaponDroneSlash );
PRECACHE_WEAPON_REGISTER( weapon_drone_slash );

acttable_t	CWeaponDroneSlash::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponDroneSlash);

#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponDroneSlash )
	DEFINE_FUNCTION( RechargeThink ),
END_DATADESC()
#endif

CWeaponDroneSlash::CWeaponDroneSlash( void ) {
}

float CWeaponDroneSlash::GetDamageForActivity( Activity hitActivity ) {
	return DRONESLASH_DAMAGE;
}

bool CWeaponDroneSlash::Deploy(void) {
	return BaseClass::Deploy();
}

void CWeaponDroneSlash::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = SharedRandomFloat( "crowbarpax", 1.0f, 2.0f );
	punchAng.y = SharedRandomFloat( "crowbarpay", -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}

void CWeaponDroneSlash::RechargeThink(void) {
#ifndef CLIENT_DLL
	CBasePlayer *owner;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	if (owner->GetAmmoCount(m_iSecondaryAmmoType) >= MAX_SPIT_COUNT) {
		SetThink(NULL);
		return;
	}

	owner->GiveAmmo(1, m_iSecondaryAmmoType, true);
	SetNextThink(gpGlobals->curtime + SPIT_RECHARGE_TIME);
#endif
}

void CWeaponDroneSlash::SecondaryAttack(void) {

	m_flNextSecondaryAttack = gpGlobals->curtime + SPIT_TIME;

	WeaponSound(SPECIAL1);

#ifndef CLIENT_DLL
	CBasePlayer *owner;
	CDroneSpit *spit;
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

	VectorAngles(fwd, angles);

	spit = spit = CDroneSpit::SpitCreate(muzzle, angles, owner);
	spit->SetAbsVelocity(fwd * SPIT_VELOCITY);

	owner->RemoveAmmo(1, m_iSecondaryAmmoType);

#endif
}

void CWeaponDroneSlash::ItemPostFrame( void ) {
	CBasePlayer *owner;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	if (owner->m_nButtons & IN_ATTACK) {
		if (m_flNextPrimaryAttack < gpGlobals->curtime) {
			PrimaryAttack();
		}
		
		return;
	}
	
	if (owner->m_nButtons & IN_ATTACK2) {
		SetThink(NULL);
		charging = false;

		if (m_flNextSecondaryAttack < gpGlobals->curtime && BaseClass::HasAnyAmmo()) {
			SecondaryAttack();
		}

		return;
	}

	WeaponIdle();

#ifndef CLIENT_DLL
	// recharge
	if (!charging && owner->GetAmmoCount(m_iSecondaryAmmoType) < MAX_SPIT_COUNT) {
		charging = true;
		SetThink(&CWeaponDroneSlash::RechargeThink);
		SetNextThink(gpGlobals->curtime + 0.4f);
	}
#endif

}


#ifndef CLIENT_DLL

void CWeaponDroneSlash::DisplayUsageHudHint(void) {
	UTIL_HudHintText(GetOwner(), "%+attack%  slash\n%+attack2%  spit");
	BaseClass::DisplayUsageHudHint();
}

void CWeaponDroneSlash::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
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


void CWeaponDroneSlash::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
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


ConVar sk_drone_slash_lead_time( "sk_drone_slash_lead_time", "0.9" );

int CWeaponDroneSlash::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_drone_slash_lead_time.GetFloat();
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


void CWeaponDroneSlash::Drop( const Vector &vecVelocity )
{
}

float CWeaponDroneSlash::GetRange( void )
{
	return	DRONESLASH_RANGE;	
}

float CWeaponDroneSlash::GetFireRate( void )
{
	return	DRONESLASH_REFIRE;	
}


