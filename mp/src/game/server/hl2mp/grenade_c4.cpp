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
#include "explode.h"
#include "grenade_c4.h"

#include "hl2mp_player.h"
#include "class_info.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FRAG_GRENADE_BLIP_FREQUENCY				1.0f
#define FRAG_GRENADE_BLIP_FAST_FREQUENCY		0.3f
#define C4_RADIUS								1800.0f

#define C4_EATEN_TIMER							10.0f

#define	C4_SPRITE								"sprites/redglow1.vmt"
#define C4_MODEL								"models/Weapons/w_slam.mdl"

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;


LINK_ENTITY_TO_CLASS( grenade_c4, CGrenadeC4 );

BEGIN_DATADESC( CGrenadeC4 )

	DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	DEFINE_THINKFUNC( ExplodeThink ),
	DEFINE_USEFUNC( Use ),

END_DATADESC()


CGrenadeC4::~CGrenadeC4( void ) {
	if ( m_hGlowSprite != NULL ) {
		UTIL_Remove( m_hGlowSprite );
		m_hGlowSprite = NULL;
	}
}

void CGrenadeC4::Use(CBaseEntity *activator, CBaseEntity *caller, USE_TYPE useType, float value) {
	CHL2MP_Player *p;

	if (use_ok == false)
		return;

	if (activator == NULL)
		return;

	p = dynamic_cast<CHL2MP_Player *>(activator);
	if (p == NULL)
		return;

	if (p->GetTeamNumber() == TEAM_HUMANS ||
		p->m_iClassNumber != CLASS_GUARDIAN_IDX) {
			return;
	}

	// can only be picked up once
	// so disable USE now
	use_ok = false;

	m_flDetonateTime = gpGlobals->curtime + C4_EATEN_TIMER;
	SetThink( &CGrenadeC4::ExplodeThink );
	SetNextThink( gpGlobals->curtime );

	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	SetAbsOrigin(activator->GetAbsOrigin() + Vector (0,0,15));

	AddEffects(EF_NODRAW);

	SetParent(activator);

	p->SwallowC4Sound();

	// emit sprites/dlight?
}

void CGrenadeC4::CreateEffects( void ) {
	// Only do this once
	if ( m_hGlowSprite != NULL )
		return;

	// Create a blinking light to show we're an active SLAM
	m_hGlowSprite = CSprite::SpriteCreate( C4_SPRITE, GetAbsOrigin(), false );
	m_hGlowSprite->SetAttachment( this, 0 );
	m_hGlowSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxStrobeFast );
	m_hGlowSprite->SetBrightness( 255, 1.0f );
	m_hGlowSprite->SetScale( 0.2f, 0.5f );
	m_hGlowSprite->TurnOn();
}

void CGrenadeC4::Detonate( void ) {
	trace_t		tr;
	Vector		vecSpot;

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

void CGrenadeC4::Explode( trace_t *pTrace, int bitsDamageType ) {

	Vector origin;
	trace_t tr;

	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 ) {
		SetAbsOrigin( pTrace->endpos + (pTrace->plane.normal * 0.6) );
	}

	ExplosionCreate( GetAbsOrigin() + Vector( 0, 0, 8 ), GetAbsAngles(), GetThrower(), GetDamage(), C4_RADIUS, 
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);

	UTIL_Remove( this );
}

void CGrenadeC4::Spawn( void ) {
	Precache();

	SetModel(C4_MODEL);

	// player overrides this
	m_flDamage		= 1200.0f;
	m_DmgRadius		= 250.0f;
	use_ok			= true;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetSolid(SOLID_BBOX);
	SetCollisionGroup(COLLISION_GROUP_NONE);
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;

	AddSolidFlags( FSOLID_NOT_STANDABLE );

	BaseClass::Spawn();

	m_hGlowSprite = NULL;
	CreateEffects();
}

void CGrenadeC4::Precache( void ) {
	PrecacheModel(C4_MODEL);
	PrecacheModel(C4_SPRITE);
	PrecacheScriptSound( "C4.Ping" );
	BaseClass::Precache();
}

void CGrenadeC4::SetTimer( float detonateDelay ) {
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	SetThink( &CGrenadeC4::ExplodeThink );
	SetNextThink( gpGlobals->curtime );
}

#ifndef CLIENT_DLL
int CGrenadeC4::ObjectCaps(void) {
	return BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
}
#endif

void CGrenadeC4::ExplodeThink()  {

	if ( gpGlobals->curtime > m_flDetonateTime ) {
		Detonate();
		return;
	}

	if ( gpGlobals->curtime > m_flNextBlipTime ) {
		EmitSound("C4.Ping");
		m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

int CGrenadeC4::OnTakeDamage( const CTakeDamageInfo &inputInfo ) {
	return BaseClass::OnTakeDamage( inputInfo );
}


CBaseGrenade *C4_Create( const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner, float timer) {
	CGrenadeC4 *pGrenade = (CGrenadeC4 *)CBaseEntity::Create( "grenade_c4", position, angles, pOwner );
	
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->SetTimer(timer);
	pGrenade->m_takedamage = DAMAGE_NO;

	return pGrenade;
}
