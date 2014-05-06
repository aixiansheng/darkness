#include "cbase.h"
#include "class_info.h"
#include "ent_infested_corpse.h"

CInfestedCorpse::CInfestedCorpse() : CBaseAnimating() {
	gib1 = "models/Gibs/Antlion_gib_small_2.mdl";
	gib2 = "models/Gibs/AGIBS.mdl";
	gib3 = "models/Gibs/Antlion_gib_small_1.mdl";
	gib4 = "models/Gibs/Antlion_gib_small_3.mdl";
	gib5 = "models/Gibs/Antlion_gib_medium_2.mdl";

#ifdef CLIENT_DLL
	flies = NULL;
#endif
}

CInfestedCorpse::~CInfestedCorpse(void) {
#ifdef CLIENT_DLL
	ParticleProp()->StopParticlesInvolving(this);
	flies = NULL;
#endif
}

LINK_ENTITY_TO_CLASS(ent_infested_corpse, CInfestedCorpse);
IMPLEMENT_NETWORKCLASS_ALIASED( InfestedCorpse, DT_InfestedCorpse )

BEGIN_NETWORK_TABLE( CInfestedCorpse, DT_InfestedCorpse )
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE()


#ifndef CLIENT_DLL

BEGIN_DATADESC(CInfestedCorpse)
	DEFINE_THINKFUNC(SoundThink),
	DEFINE_THINKFUNC(DamageThink),
END_DATADESC()

// server-side class stuff
void CInfestedCorpse::Event_Killed(const CTakeDamageInfo &info) {
	Vector origin;
	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;

	if (GetParametersForSound( CORPSE_GIB_SOUND, params, NULL )) {
		origin = GetAbsOrigin();

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

	if (gib1 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib1, 10);
	if (gib2 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib2, 10);
	if (gib3 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib3, 15);
	if (gib4 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib4, 10);
	if (gib5 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib5, 15);

	BaseClass::Event_Killed(info);
	
	StopParticleEffects(this);

	UTIL_Remove(this);
}

void CInfestedCorpse::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	SetModel(CORPSE_MODEL);
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY);
	AddSolidFlags(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS | FSOLID_NOT_STANDABLE | FSOLID_NOT_SOLID);
	SetCollisionGroup(COLLISION_GROUP_PLAYER);

	SetSequence(LookupSequence("idle"));
	SetPlaybackRate(1.0f);
	UseClientSideAnimation();

	SetMaxHealth(CORPSE_HEALTH);
	SetHealth(CORPSE_HEALTH);

	m_takedamage = DAMAGE_YES;

	SetThink(&CInfestedCorpse::DamageThink);
	SetNextThink(gpGlobals->curtime + CORPSE_DMG_INT);

	RegisterThinkContext(CORPSE_SND_CTX);
	SetContextThink
	(
		&CInfestedCorpse::SoundThink,
		gpGlobals->curtime + CORPSE_SND_INT,
		CORPSE_SND_CTX
	);
	
	ChangeTeam(TEAM_SPIDERS);
}

CHL2MP_Player *CInfestedCorpse::GetCreator(void) const {
	return dynamic_cast<CHL2MP_Player *>(creator.Get());
}

CHL2MP_Player *CInfestedCorpse::GetCreator(void) {
	return dynamic_cast<CHL2MP_Player *>(creator.Get());
}

void CInfestedCorpse::SetCreator(CHL2MP_Player *player) {
	creator = player;
}

void CInfestedCorpse::Precache(void) {
	PrecacheScriptSound(CORPSE_GIB_SOUND);
	PrecacheModel(CORPSE_MODEL);
	PrecacheParticleSystem(CORPSE_FLIES_SPRITE);
	PrecacheScriptSound(CORPSE_FLIES_SOUND);

	if (gib1)
		PrecacheModel(gib1);
	if (gib2)
		PrecacheModel(gib2);
	if (gib3)
		PrecacheModel(gib3);
	if (gib4)
		PrecacheModel(gib4);
	if (gib5)
		PrecacheModel(gib5);

	BaseClass::Precache();
}

void CInfestedCorpse::SoundThink(void) {
	Vector origin;
	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;
	
	SetNextThink(gpGlobals->curtime + CORPSE_SND_INT, CORPSE_SND_CTX);

	if (GetParametersForSound( CORPSE_FLIES_SOUND, params, NULL )) {
		origin = GetAbsOrigin();

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
}

void CInfestedCorpse::DamageThink(void) {
	CBaseEntity *ent;
	trace_t tr;
	
	for (CEntitySphereQuery sphere(GetAbsOrigin(), CORPSE_DMG_RAD); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		UTIL_TraceLine
		(
			GetAbsOrigin(),
			ent->GetAbsOrigin(),
			MASK_SHOT,
			this,
			COLLISION_GROUP_NONE,
			&tr
		);
		
		if (tr.DidHit() && tr.m_pEnt != ent)
			continue; // something's in the way

		if (ent->IsPlayer() && ent->GetTeamNumber() == TEAM_HUMANS && ent->IsAlive()) {
			CTakeDamageInfo info(this, this, CORPSE_DMG_VAL, DMG_ACID);
			ent->DispatchTraceAttack(info, Vector(0,0,1), &tr);
			ApplyMultiDamage();
		}
	}

	SetNextThink(gpGlobals->curtime + CORPSE_DMG_INT);
}



#else

int CInfestedCorpse::DrawModel(int flags) {
	C_HL2MP_Player *local;

	shouldGlow = false;

	local = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (local && local->GetTeamNumber() == TEAM_SPIDERS) {
		shouldGlow = true;
		if (GetTeamNumber() == TEAM_HUMANS && local->m_iClassNumber == CLASS_COMMANDO_IDX
			&& GetAbsVelocity() == vec3_origin) {
				shouldGlow = false;
		}
	}

	return BaseClass::DrawModel(flags);
}

void CInfestedCorpse::OnDataChanged(DataUpdateType_t type) {
	BaseClass::OnDataChanged(type);
	if (type == DATA_UPDATE_CREATED) {
		ShowFlies();
		return;
	}
}

void CInfestedCorpse::ShowFlies(void) {
	if (flies == NULL) {
		flies = ParticleProp()->Create(CORPSE_FLIES_SPRITE, PATTACH_ABSORIGIN_FOLLOW);
	}
}

void CInfestedCorpse::UpdateOnRemove(void) {
	if (flies != NULL) {
		StopParticleEffects(this);
		ParticleProp()->StopParticlesInvolving(this);
		flies = NULL;
	}
}

#endif