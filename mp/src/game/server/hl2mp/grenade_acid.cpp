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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FRAG_GRENADE_BLIP_FREQUENCY			1.0f
#define FRAG_GRENADE_BLIP_FAST_FREQUENCY	0.3f

#define FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP 1.5f
#define FRAG_GRENADE_WARN_TIME 1.5f

#define ACID_TIME	3.25f
#define ACID_DAMAGE	40.0f

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

#define GRENADE_MODEL "models/Gibs/HGIBS.mdl"

class CGrenadeAcid : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeAcid, CBaseGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
	~CGrenadeAcid( void );

public:
	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	void	SetTimer( float detonateDelay, float warnDelay );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	BlipSound() { EmitSound( "Grenade.Blip" ); }
	void	DelayThink(void);
	void	SpewThink(void);
	void	VPhysicsUpdate( IPhysicsObject *pPhysics );
	void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	void	SetCombineSpawned( bool combineSpawned ) { m_combineSpawned = combineSpawned; }
	bool	IsCombineSpawned( void ) const { return m_combineSpawned; }
	void	SetPunted( bool punt ) { m_punted = punt; }
	bool	WasPunted( void ) const { return m_punted; }
	void	Detonate(void);
	void	Explode( trace_t *pTrace, int bitsDamageType );

	// this function only used in episodic.
#ifdef HL2_EPISODIC
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
#endif 

	void	InputSetTimer( inputdata_t &inputdata );

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	m_combineSpawned;
	bool	m_punted;

	float m_flSpewEndTime;
};

LINK_ENTITY_TO_CLASS( grenade_acid, CGrenadeAcid );

BEGIN_DATADESC( CGrenadeAcid )

	// Fields
	DEFINE_FIELD( m_pMainGlow, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_combineSpawned, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_punted, FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_THINKFUNC( DelayThink ),
	DEFINE_THINKFUNC( SpewThink ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTimer", InputSetTimer ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeAcid::~CGrenadeAcid( void ) {}

void CGrenadeAcid::Detonate( void ) {
	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, & tr);

	if( tr.startsolid ) {
		// Since we blindly moved the explosion origin vertically, we may have inadvertently moved the explosion into a solid,
		// in which case nothing is going to be harmed by the grenade's explosion because all subsequent traces will startsolid.
		// If this is the case, we do the downward trace again from the actual origin of the grenade. (sjb) 3/8/2007  (for ep2_outland_09)
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );
	}

	Explode( &tr, DMG_ACID );

	SetThink( &CGrenadeAcid::SpewThink );
	SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeAcid::Explode( trace_t *pTrace, int bitsDamageType ) {
	//
	// In this case, start spewing acid
	// SpewThink will turn it off when it's done
	//

#if !defined( CLIENT_DLL )
	
	// SetModelName( NULL_STRING );//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 ) {
		SetAbsOrigin( pTrace->endpos + (pTrace->plane.normal * 0.6) );
	}

	// Vector vecAbsOrigin = GetAbsOrigin();
	// CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );
	// Use the thrower's position as the reported position

	SetTouch( NULL );
	SetSolid( SOLID_NONE );
	SetAbsVelocity( vec3_origin );
	SetMoveType(MOVETYPE_NONE);

#endif
	
	m_flSpewEndTime = gpGlobals->curtime + ACID_TIME;

	DispatchParticleEffect("acid_gren", GetAbsOrigin() + Vector(0,0,10), GetAbsAngles(), this);
}

void CGrenadeAcid::Spawn( void )
{
	Precache( );

	SetModel( GRENADE_MODEL );

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CreateVPhysics();

	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;

	AddSolidFlags( FSOLID_NOT_STANDABLE );

	m_combineSpawned	= false;
	m_punted			= false;

	BaseClass::Spawn();

	m_flDamage		= 5.0f;
	m_DmgRadius		= 250.0f;
	m_takedamage	= DAMAGE_NO;
	m_iHealth		= 1;
}

bool CGrenadeAcid::CreateVPhysics() {
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );
	
	CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( pEntity )
		{
			if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
				return false;
			if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

void CGrenadeAcid::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity( &vel, &angVel );
	
	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
	UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
	if ( tr.startsolid )
	{
		if ( !m_inSolid )
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity( &vel, NULL );
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if ( tr.DidHit() )
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
		tr.m_pEnt->TakeDamage( info );

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;
		
		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity( &vel, &angVel );
	}
}


void CGrenadeAcid::Precache( void )
{
	PrecacheModel( GRENADE_MODEL );

	PrecacheScriptSound( "Grenade.Blip" );

	PrecacheModel( "sprites/redglow1.vmt" );
	PrecacheModel( "sprites/bluelaser1.vmt" );

	PrecacheParticleSystem("acid_gren");

	BaseClass::Precache();
}

void CGrenadeAcid::SetTimer( float detonateDelay, float warnDelay ) {
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink( &CGrenadeAcid::DelayThink );
	SetNextThink( gpGlobals->curtime );
}

void CGrenadeAcid::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	SetThrower( pPhysGunUser );

#ifdef HL2MP
	SetTimer( FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP, FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP / 2);

	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
	m_bHasWarnedAI = true;
#else
	if( IsX360() )
	{
		// Give 'em a couple of seconds to aim and throw. 
		SetTimer( 2.0f, 1.0f);
		BlipSound();
		m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
	}
#endif

#ifdef HL2_EPISODIC
	SetPunted( true );
#endif

	BaseClass::OnPhysGunPickup( pPhysGunUser, reason );
}

void CGrenadeAcid::SpewThink(void) {
	CBaseEntity *thrower;

	if (gpGlobals->curtime > m_flSpewEndTime) {
		StopParticleEffects(this);
		SetThink( &CBaseGrenade::SUB_Remove );
		AddEffects( EF_NODRAW );
		SetNextThink( gpGlobals->curtime );
	}

	thrower = GetThrower();
	if (thrower == NULL) {
		thrower = GetOwnerEntity();
	}

	Vector vecReported = thrower ? thrower->GetAbsOrigin() : vec3_origin;
	CTakeDamageInfo info( this, thrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, DMG_ACID);
	RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	SetNextThink(gpGlobals->curtime + 0.1);
}

void CGrenadeAcid::DelayThink() 
{
	if( gpGlobals->curtime > m_flDetonateTime ) {
		Detonate();
		return;
	}

	if( !m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime ) {
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this );
#endif
		m_bHasWarnedAI = true;
	}
	
	if( gpGlobals->curtime > m_flNextBlipTime ) {
		BlipSound();
		
		if( m_bHasWarnedAI ) {
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
		} else {
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;
		}
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeAcid::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CGrenadeAcid::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

#ifdef HL2_EPISODIC
extern int	g_interactionBarnacleVictimGrab; ///< usually declared in ai_interactions.h but no reason to haul all of that in here.
extern int g_interactionBarnacleVictimBite;
extern int g_interactionBarnacleVictimReleased;
bool CGrenadeAcid::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
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

void CGrenadeAcid::InputSetTimer( inputdata_t &inputdata )
{
	SetTimer( inputdata.value.Float(), inputdata.value.Float() - FRAG_GRENADE_WARN_TIME );
}

CBaseGrenade *AcidGren_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeAcid *pGrenade = (CGrenadeAcid *)CBaseEntity::Create( "grenade_acid", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );

	return pGrenade;
}

bool AcidGren_WasPunted( const CBaseEntity *pEntity )
{
	const CGrenadeAcid *pAcid = dynamic_cast<const CGrenadeAcid *>( pEntity );
	if ( pAcid ) {
		return pAcid->WasPunted();
	}

	return false;
}

bool AcidGren_WasCreatedByCombine( const CBaseEntity *pEntity )
{
	const CGrenadeAcid *pAcid = dynamic_cast<const CGrenadeAcid *>( pEntity );
	if ( pAcid ) {
		return pAcid->IsCombineSpawned();
	}

	return false;
}
