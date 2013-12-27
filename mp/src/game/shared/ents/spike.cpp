#include "cbase.h"
#include "spike.h"
#include "hl2mp_gamerules.h"
#include "ammodef.h"

#ifndef CLIENT_DLL
#include "world.h"
#endif

#define SPIKE_HIT_SOUND "Weapon_Crossbow.BoltHitWorld"

LINK_ENTITY_TO_CLASS( ent_spike, CSpike );

//BEGIN_DATADESC( CSpike )
//END_DATADESC()

#ifndef CLIENT_DLL


CSpike::~CSpike(void) {}

void CSpike::SpikeTouch(CBaseEntity *other) {
	trace_t tr;
	Vector forward;

	if (GetOwnerEntity() && GetOwnerEntity()->edict() == other->edict())
		return;

	if (other->ClassMatches("ent_spike") || other->ClassMatches("ent_spiker")) {
		return;
	}

	if (other->GetTeamNumber() == TEAM_HUMANS) {
		tr = BaseClass::GetTouchTrace();
		forward = tr.endpos - tr.startpos;

		CDisablePredictionFiltering foo;

		// opt for some fleshy hit sound...
		// EmitSound(SPIKE_HIT_SOUND);

		SetSolid(SOLID_NONE);

		CTakeDamageInfo info(this, GetOwnerEntity(), GetAbsVelocity(), GetAbsOrigin(), SPIKE_DAMAGE, DMG_SLASH | DMG_ALWAYSGIB);
		CalculateMeleeDamageForce(&info, forward, tr.endpos, 0.5f);
		other->DispatchTraceAttack(info, forward, &tr);
		ApplyMultiDamage();

		UTIL_Remove( this );
		return;
	}

	// allow it to pass through water
	if (other->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;

	EmitSound(SPIKE_HIT_SOUND);

	UTIL_Remove(this);
}

void CSpike::Precache(void) {
	PrecacheModel(SPIKE_MODEL);
	PrecacheScriptSound(SPIKE_HIT_SOUND);
	BaseClass::Precache();	
}

void CSpike::Spawn(void) {
	Precache();
	SetModel(SPIKE_MODEL);

	ChangeTeam(TEAM_SPIDERS);

	m_takedamage = DAMAGE_NO;

	UTIL_SetSize(this, -Vector(5,5,5), Vector(5,5,5));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_NONE);
	SetMoveType(MOVETYPE_NONE);
	BaseClass::Spawn();

	AddEffects(EF_NOSHADOW);
}

void CSpike::FireAt(Vector v) {
	SetParent(NULL);
	SetAbsVelocity(v);
	SetMoveType(MOVETYPE_FLY);
	SetTouch(&CSpike::SpikeTouch);
}

CSpike *Spike_Create( const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner, float damage, bool fire_now) {
	CSpike *s = (CSpike *)CBaseEntity::Create( "ent_spike", position, angles, pOwner );

	if (fire_now) {
		s->FireAt(velocity);
	}

	return s;
}

#endif