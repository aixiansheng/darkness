//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "grenade_frag.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "soundent.h"
#include "particle_parse.h"
#include "hl2mp_gamerules.h"
#include "ammodef.h"
#include "grenade_guardian.h"

#ifndef CLIENT_DLL
	#include "explode.h"
	#include "te_effect_dispatch.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FRAG_GRENADE_BLIP_FREQUENCY			1.0f
#define FRAG_GRENADE_BLIP_FAST_FREQUENCY	0.3f

#define FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP 1.5f
#define FRAG_GRENADE_WARN_TIME 1.5f

#define GUARDIAN_TIME	4.0f
#define SPIKE_GREN_SHOT_DMG	50.0f

#define GRENADE_DETECT_RADIUS		100
#define STICKY_DELAY	1.0f

#define SPIKE_GREN_AMMO_TYPE "spike"
#define SPIKE_GREN_SHOT_SOUND "Weapon_Crossbow.Single"

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

#define GUARDIAN_GRENADE_MODEL "models/spike_ball.mdl"

#define SPIKE_GREN_DMG 60
#define SPIKE_GREN_RAD 60

LINK_ENTITY_TO_CLASS( grenade_guardian, CGrenadeGuardian );

BEGIN_DATADESC( CGrenadeGuardian )

	DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	DEFINE_THINKFUNC( ExplodeThink ),
	DEFINE_THINKFUNC( TriggerThink ),

END_DATADESC()


CGrenadeGuardian::~CGrenadeGuardian( void ) {}

void CGrenadeGuardian::StickTouch(CBaseEntity *other) {
	trace_t tr;
	
	if (!other->IsSolid() || 
		other->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) || 
		other == GetThrower() ||
		other->GetMoveType() != MOVETYPE_NONE) {
		
		// ignore invalid touches
		return;
	}

	surface_trace = BaseClass::GetTouchTrace();

	// if it's thrown into a sky, it should go away
	if (surface_trace.surface.flags & SURF_SKY) {
		SetThink(NULL);
		UTIL_Remove(this);
		return;
	}

	//
	// we actually hit something solid... stick to it
	//

	BlipSound();
	SetMoveType(MOVETYPE_NONE);
	SetTouch(NULL);
	
	SetThink(&CGrenadeGuardian::TriggerThink);
	SetNextThink(gpGlobals->curtime + STICKY_DELAY);
}

//
// this method differentiates a guardian sticky spike
// grenade from a regular stalker spike grenade
//

void CGrenadeGuardian::TriggerThink(void) {
	CBaseEntity *ent;
	trace_t tr;

	SetNextThink(gpGlobals->curtime + 0.1f);
	
	for (CEntitySphereQuery sphere(GetAbsOrigin(), GRENADE_DETECT_RADIUS); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		UTIL_TraceLine(GetAbsOrigin(), ent->GetAbsOrigin(), MASK_SHOT, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr);
		
		if (tr.DidHit() && tr.m_pEnt != ent)
			continue;

		if (ent->IsPlayer() && ent->GetTeamNumber() == TEAM_HUMANS) {
			Detonate();
		}
	}
}


void CGrenadeGuardian::SetSticky(void) {
	sticky = true;
	SetTouch(&CGrenadeGuardian::StickTouch);
}

void CGrenadeGuardian::Detonate( void ) {
	trace_t		tr;
	Vector		vecSpot;

	if (sticky) {

		Explode(&surface_trace, DMG_BULLET);

	} else {

		vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
		UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, & tr);
		
		if ( tr.startsolid ) {
			// Since we blindly moved the explosion origin vertically, we may have inadvertently moved the explosion into a solid,
			// in which case nothing is going to be harmed by the grenade's explosion because all subsequent traces will startsolid.
			// If this is the case, we do the downward trace again from the actual origin of the grenade. (sjb) 3/8/2007  (for ep2_outland_09)
			UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );
		}

		Explode( &tr, DMG_BULLET );

	}

	
}

#define GREN_VEC3_UP	Vector(0,0,10)
#define GREN_VEC3_DN	Vector(0,0,-10)

// flat on Z
#define GREN_VEC3_N		Vector(0,10,0)
#define GREN_VEC3_NE	Vector(7,7,0)
#define GREN_VEC3_E		Vector(10,0,0)
#define GREN_VEC3_SE	Vector(7,-7,0)
#define GREN_VEC3_S		Vector(0,-10,0)
#define GREN_VEC3_SW	Vector(-7,-7,0)
#define GREN_VEC3_W		Vector(-10,0,0)
#define GREN_VEC3_NW	Vector(-7,7,0)

// elevated 45 deg
#define GREN_VEC3_UP_N	Vector(0,7,7)
#define GREN_VEC3_UP_NE	Vector(5,5,5)
#define GREN_VEC3_UP_E	Vector(7,0,7)
#define GREN_VEC3_UP_SE	Vector(5,-5,5)
#define GREN_VEC3_UP_S	Vector(0,-7,7)
#define GREN_VEC3_UP_SW	Vector(-5,-5,5)
#define GREN_VEC3_UP_W	Vector(-7,0,7)
#define GREN_VEC3_UP_NW	Vector(-5,5,5)

// declined 45 deg
#define GREN_VEC3_DN_N	Vector(0,7,-7)
#define GREN_VEC3_DN_NE	Vector(5,5,-5)
#define GREN_VEC3_DN_E	Vector(7,0,-7)
#define GREN_VEC3_DN_SE	Vector(5,-5,-5)
#define GREN_VEC3_DN_S	Vector(0,-7,-7)
#define GREN_VEC3_DN_SW	Vector(-5,-5,-5)
#define GREN_VEC3_DN_W	Vector(-7,0,-7)
#define GREN_VEC3_DN_NW	Vector(-5,5,-5)

// elevated less than 45 deg
#define GREN_VEC3_UP_N_22	Vector(0,7,3)
#define GREN_VEC3_UP_NE_22	Vector(5,5,2)
#define GREN_VEC3_UP_E_22	Vector(7,0,3)
#define GREN_VEC3_UP_SE_22	Vector(5,-5,2)
#define GREN_VEC3_UP_S_22	Vector(0,-7,3)
#define GREN_VEC3_UP_SW_22	Vector(-5,-5,2)
#define GREN_VEC3_UP_W_22	Vector(-7,0,3)
#define GREN_VEC3_UP_NW_22	Vector(-5,5,2)

// declined less than 45 deg
#define GREN_VEC3_DN_N_22	Vector(0,7,-3)
#define GREN_VEC3_DN_NE_22	Vector(5,5,-2)
#define GREN_VEC3_DN_E_22	Vector(7,0,-3)
#define GREN_VEC3_DN_SE_22	Vector(5,-5,-2)
#define GREN_VEC3_DN_S_22	Vector(0,-7,-3)
#define GREN_VEC3_DN_SW_22	Vector(-5,-5,-2)
#define GREN_VEC3_DN_W_22	Vector(-7,0,-3)
#define GREN_VEC3_DN_NW_22	Vector(-5,5,-2)

void CGrenadeGuardian::Disappear(void) {
	UTIL_Remove(this);
}

void CGrenadeGuardian::Explode( trace_t *pTrace, int bitsDamageType ) {
	Vector origin;
	trace_t tr;
	
	SetTouch(NULL);
	SetNextThink(gpGlobals->curtime + 1.0f);
	SetThink(&CGrenadeGuardian::Disappear);
	SetSolid(SOLID_NONE);
	SetAbsVelocity(vec3_origin);

	SetModelName(NULL_STRING);
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if (pTrace->fraction != 1.0) {
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	origin = GetAbsOrigin();

	// shoot spikes in many directions, but not at nearby walls
	
	ShootSpike(origin, GREN_VEC3_UP);
	ShootSpike(origin, GREN_VEC3_DN);

	ShootSpike(origin, GREN_VEC3_N);
	ShootSpike(origin, GREN_VEC3_NE);
	ShootSpike(origin, GREN_VEC3_E);
	ShootSpike(origin, GREN_VEC3_SE);
	ShootSpike(origin, GREN_VEC3_S);
	ShootSpike(origin, GREN_VEC3_SW);
	ShootSpike(origin, GREN_VEC3_W);
	ShootSpike(origin, GREN_VEC3_NW);

	ShootSpike(origin, GREN_VEC3_UP_N);
	ShootSpike(origin, GREN_VEC3_UP_NE);
	ShootSpike(origin, GREN_VEC3_UP_E);
	ShootSpike(origin, GREN_VEC3_UP_SE);
	ShootSpike(origin, GREN_VEC3_UP_S);
	ShootSpike(origin, GREN_VEC3_UP_SW);
	ShootSpike(origin, GREN_VEC3_UP_W);
	ShootSpike(origin, GREN_VEC3_UP_NW);

	ShootSpike(origin, GREN_VEC3_DN_N);
	ShootSpike(origin, GREN_VEC3_DN_NE);
	ShootSpike(origin, GREN_VEC3_DN_E);
	ShootSpike(origin, GREN_VEC3_DN_SE);
	ShootSpike(origin, GREN_VEC3_DN_S);
	ShootSpike(origin, GREN_VEC3_DN_SW);
	ShootSpike(origin, GREN_VEC3_DN_W);
	ShootSpike(origin, GREN_VEC3_DN_NW);

	ShootSpike(origin, GREN_VEC3_UP_N_22);
	ShootSpike(origin, GREN_VEC3_UP_NE_22);
	ShootSpike(origin, GREN_VEC3_UP_E_22);
	ShootSpike(origin, GREN_VEC3_UP_SE_22);
	ShootSpike(origin, GREN_VEC3_UP_S_22);
	ShootSpike(origin, GREN_VEC3_UP_SW_22);
	ShootSpike(origin, GREN_VEC3_UP_W_22);
	ShootSpike(origin, GREN_VEC3_UP_NW_22);

	ShootSpike(origin, GREN_VEC3_DN_N_22);
	ShootSpike(origin, GREN_VEC3_DN_NE_22);
	ShootSpike(origin, GREN_VEC3_DN_E_22);
	ShootSpike(origin, GREN_VEC3_DN_SE_22);
	ShootSpike(origin, GREN_VEC3_DN_S_22);
	ShootSpike(origin, GREN_VEC3_DN_SW_22);
	ShootSpike(origin, GREN_VEC3_DN_W_22);
	ShootSpike(origin, GREN_VEC3_DN_NW_22);

	int exflags = \
		SF_ENVEXPLOSION_NOFIREBALL |
		SF_ENVEXPLOSION_NOSMOKE |
		SF_ENVEXPLOSION_NOSPARKS |
		SF_ENVEXPLOSION_NOSOUND |
		SF_ENVEXPLOSION_NOFIREBALLSMOKE |
		SF_ENVEXPLOSION_NOPARTICLES |
		SF_ENVEXPLOSION_NODLIGHTS |
		SF_ENVEXPLOSION_SURFACEONLY |
		SF_ENVEXPLOSION_GENERIC_DAMAGE;
			
	ExplosionCreate
	(
		origin,
		GetAbsAngles(),
		GetThrower(),
		SPIKE_GREN_DMG,
		SPIKE_GREN_RAD,
		exflags,
		0.0f,
		this
	);


	// CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );
	// Use the thrower's position as the reported position
}

void CGrenadeGuardian::ShootSpike(Vector origin, Vector v) {
	int ammotype;
	trace_t tr;
	FireBulletsInfo_t info;
	CSoundParameters params;
	CRecipientFilter filter;
	CDisablePredictionFiltering pfilter;
	EmitSound_t ep;
	
	ammotype = GetAmmoDef()->Index("grenade_guardian");

	if (GetParametersForSound(SPIKE_GREN_SHOT_SOUND, params, NULL)) {
		filter.AddRecipientsByPAS( origin );

		ep.m_nChannel = params.channel;
		ep.m_pSoundName = params.soundname;
		ep.m_flVolume = params.volume;
		ep.m_SoundLevel = params.soundlevel;
		ep.m_nFlags = 0;
		ep.m_nPitch = params.pitch;
		ep.m_pOrigin = &origin;

		EmitSound( filter, entindex(), ep );
	}

	info.m_iShots = 1;
	info.m_vecSrc = origin;
	info.m_vecDirShooting = v;
	info.m_flDistance = 512.0;
	info.m_iAmmoType = ammotype;
	info.m_iTracerFreq = 4;
	info.m_pAttacker = GetThrower();
	info.m_flDamage = SPIKE_GREN_SHOT_DMG;
	FireBullets(info);

}

void CGrenadeGuardian::Spawn( void )
{
	Precache( );

	SetModel( GUARDIAN_GRENADE_MODEL );

	m_flDamage		= 0.0f;
	m_DmgRadius		= 250.0f;
	sticky			= false;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetSolid(SOLID_BBOX);
	SetCollisionGroup(COLLISION_GROUP_NONE);
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;

	AddSolidFlags( FSOLID_NOT_STANDABLE );

	BaseClass::Spawn();
}

void CGrenadeGuardian::Precache( void ) {
	PrecacheModel( GUARDIAN_GRENADE_MODEL );
	PrecacheScriptSound( "Grenade.Blip" );
	PrecacheScriptSound(SPIKE_GREN_SHOT_SOUND);
	BaseClass::Precache();
}

void CGrenadeGuardian::SetTimer( float detonateDelay ) {
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	SetThink( &CGrenadeGuardian::ExplodeThink );
	SetNextThink( gpGlobals->curtime );
}

void CGrenadeGuardian::ExplodeThink()  {
	if ( gpGlobals->curtime > m_flDetonateTime ) {
		Detonate();
		return;
	}

	if ( gpGlobals->curtime > m_flNextBlipTime ) {
		BlipSound();
		
		m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

int CGrenadeGuardian::OnTakeDamage( const CTakeDamageInfo &inputInfo ) {
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	// VPhysicsTakeDamage( inputInfo );
	return BaseClass::OnTakeDamage( inputInfo );
}

#ifdef HL2_EPISODIC
extern int	g_interactionBarnacleVictimGrab; ///< usually declared in ai_interactions.h but no reason to haul all of that in here.
extern int g_interactionBarnacleVictimBite;
extern int g_interactionBarnacleVictimReleased;
bool CGrenadeGuardian::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	// allow fragnades to be grabbed by barnacles. 
	if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		// give the grenade another five seconds seconds so the player can have the satisfaction of blowing up the barnacle with it
		float timer = m_flDetonateTime - gpGlobals->curtime + 5.0f;
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );

		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimBite )
	{
		// detonate the grenade immediately 
		SetTimer( 0, 0 );
		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimReleased )
	{
		// take the five seconds back off the timer.
		float timer = max(m_flDetonateTime - gpGlobals->curtime - 5.0f,0.0f);
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
		return true;
	}
	else
	{
		return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
	}
}
#endif


CBaseGrenade *GuardianGren_Create( const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner, float timer, bool sticky)
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeGuardian *pGrenade = (CGrenadeGuardian *)CBaseEntity::Create( "grenade_guardian", position, angles, pOwner );
	
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	
	if (sticky) {
		pGrenade->m_takedamage = DAMAGE_YES;
		pGrenade->m_iHealth = 20;
		pGrenade->SetSticky();
	} else {
		pGrenade->m_takedamage = DAMAGE_NO;
		pGrenade->SetTimer( timer );
	}

	return pGrenade;
}
