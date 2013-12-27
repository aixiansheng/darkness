#include "cbase.h"
#include "spiker.h"
#include "spike.h"
#include "item_info.h"
#include "hl2mp_gamerules.h"

#define SPEED_FACTOR 1200

LINK_ENTITY_TO_CLASS(ent_spiker, CSpikerEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( SpikerEntity, DT_SpikerEntity );

BEGIN_NETWORK_TABLE( CSpikerEntity, DT_SpikerEntity )
END_NETWORK_TABLE()

CSpikerEntity::CSpikerEntity() : CSpiderMateriel(&spider_items[ITEM_SPIKER_IDX]) {
}

CSpikerEntity::~CSpikerEntity() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CSpikerEntity)
	DEFINE_THINKFUNC(DetectThink),
	DEFINE_THINKFUNC(SetupThink),
	DEFINE_THINKFUNC(RechargeThink),
END_DATADESC()

void CSpikerEntity::Precache(void) {
	BaseClass::Precache();
	PrecacheScriptSound(SPIKER_INIT_SOUND);
}

void CSpikerEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	UTIL_SetSize(this, SPIKER_HULL_MIN, SPIKER_HULL_MAX);

	SetThink(&CSpikerEntity::SetupThink);
	SetNextThink(gpGlobals->curtime + SPIKER_ARM_TIME);

	ammotype = GetAmmoDef()->Index( SPIKER_AMMO_TYPE );
	
	s1 = NULL;
	s2 = NULL;
	s3 = NULL;
	s4 = NULL;
}

void CSpikerEntity::SetSpikes(void) {
	Vector vec;
	QAngle ang;

	s1 = NULL;
	s2 = NULL;
	s3 = NULL;
	s4 = NULL;

	if (GetAttachment("arm1", vec, ang)) {
		s1 =  Spike_Create(vec, ang, Vector(0,0,0), this, SPIKER_DAMAGE, false);
		s1->SetParent(this);
	}
	
	if (GetAttachment("arm2", vec, ang)) {
		s2 =  Spike_Create(vec, ang, Vector(0,0,0), this, SPIKER_DAMAGE, false);
		s2->SetParent(this);
	}
	
	if (GetAttachment("arm3", vec, ang)) {
		s3 =  Spike_Create(vec, ang, Vector(0,0,0), this, SPIKER_DAMAGE, false);
		s3->SetParent(this);
	}
	
	if (GetAttachment("arm4", vec, ang)) {
		s4 =  Spike_Create(vec, ang, Vector(0,0,0), this, SPIKER_DAMAGE, false);
		s4->SetParent(this);
	}

}

void CSpikerEntity::DetectThink(void) {
	CBaseEntity *ent;
	trace_t tr;

	SetNextThink(gpGlobals->curtime + SPIKER_THINK_INTERVAL);
	
	for (CEntitySphereQuery sphere(GetAbsOrigin(), SPIKER_RADIUS / 2); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		UTIL_TraceLine(GetAbsOrigin(), ent->GetAbsOrigin(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		
		if (tr.DidHit() && tr.m_pEnt != ent)
			continue; // something's in the way

		if (ent->IsPlayer() && ent->GetTeamNumber() == TEAM_HUMANS && ent->IsAlive()) {
			Explode(ent, DMG_BULLET);
			return;
		}
	}
}

void CSpikerEntity::Shoot4Spikes(void) {
	Vector origin, fwd, right, up;

	if (s1) {
		if (GetAttachment("arm1", origin, &fwd, &right, &up)) {
			s1->FireAt(fwd * SPEED_FACTOR);
		} else {
			UTIL_Remove(s1);
		}
	}

	if (s2) {
		if (GetAttachment("arm2", origin, &fwd, &right, &up)) {
			s2->FireAt(fwd * SPEED_FACTOR);
		} else {
			UTIL_Remove(s2);
		}
	}
	
	if (s3) {
		if (GetAttachment("arm3", origin, &fwd, &right, &up)) {
			s3->FireAt(fwd * SPEED_FACTOR);
		} else {
			UTIL_Remove(s3);
		}
	}

	if (s4) {
		if (GetAttachment("arm4", origin, &fwd, &right, &up)) {
			s4->FireAt(fwd * SPEED_FACTOR);
		} else {
			UTIL_Remove(s4);
		}
	}

	s1 = NULL;
	s2 = NULL;
	s3 = NULL;
	s4 = NULL;

}

void CSpikerEntity::SetupThink(void) {
	SetThink(&CSpikerEntity::DetectThink);
	SetNextThink(gpGlobals->curtime + SPIKER_THINK_INTERVAL);
	
	SetSpikes();
}

void CSpikerEntity::Event_Killed(const CTakeDamageInfo &info) {
	Shoot4Spikes();
	BaseClass::Event_Killed(info);
}

void CSpikerEntity::Explode( CBaseEntity *ent, int bitsDamageType ) {

	Vector origin;
	Vector playerOrigin;
	Vector toPlayer;

	playerOrigin = ent->GetAbsOrigin();
	origin = GetAbsOrigin();
	playerOrigin += Vector(0,0,32);

	toPlayer = playerOrigin - origin;
	VectorNormalize(toPlayer);

	Shoot4Spikes();
	ShootProjectileSpike(origin, toPlayer);

	SetThink(&CSpikerEntity::RechargeThink);
	SetNextThink(gpGlobals->curtime + SPIKER_RECHARGE_TIME);
}

void CSpikerEntity::RechargeThink(void) {
	SetThink(&CSpikerEntity::DetectThink);
	SetNextThink(gpGlobals->curtime + SPIKER_THINK_INTERVAL);
	
	SetSpikes();
}

void CSpikerEntity::ShootProjectileSpike(Vector origin, Vector v) {
	QAngle ang;

	VectorAngles(v, ang);

	CSpike *s = Spike_Create(origin, ang, v * 1000, this, SPIKER_DAMAGE, true);
	(void)s;
}

void CSpikerEntity::ShootSpike(Vector origin, Vector v) {
	trace_t tr;

	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;
	
	if (GetParametersForSound( SPIKER_SHOT_SOUND, params, NULL )) {
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

	UTIL_TraceLine(origin, origin + v, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
	if (!tr.DidHitWorld() && tr.m_pEnt != NULL) {
		CTakeDamageInfo info(this, this, SPIKER_DAMAGE, DMG_BULLET);
		CalculateMeleeDamageForce(&info, v, tr.endpos);
		tr.m_pEnt->DispatchTraceAttack(info, v, &tr);
		ApplyMultiDamage();
	}
}

#endif
