#include "cbase.h"
#include "egg.h"
#include "item_info.h"

LINK_ENTITY_TO_CLASS(ent_egg, CEggEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( EggEntity, DT_EggEntity );

BEGIN_NETWORK_TABLE( CEggEntity, DT_EggEntity )
END_NETWORK_TABLE()

#define NOCLIP_RADIUS		40
#define NOCLIP_THINK_TIME	0.3f

CEggEntity::CEggEntity() : CSpiderMateriel(&dk_items[ITEM_EGG_IDX]) {
#ifndef CLIENT_DLL
	noclip_ents.Purge();
	spawnpoint = (CTeamSpawnPoint *)CreateEntityByName("info_player_teamspawn");
#endif
}

CEggEntity::~CEggEntity() {
#ifndef CLIENT_DLL
	noclip_ents.Purge();
	if (spawnpoint) {
		UTIL_Remove(spawnpoint);
	}
#endif
}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CEggEntity)
	DEFINE_FIELD(active, FIELD_BOOLEAN),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
	DEFINE_THINKFUNC(NoClipThink),
END_DATADESC()

void CEggEntity::Precache(void) {
	BaseClass::Precache();
	PrecacheScriptSound(EGG_SPAWN_PLAYER_SOUND);
}

void CEggEntity::Spawn(void) {
	CTeam *t;

	Precache();	
	BaseClass::Spawn();

	UTIL_SetSize(this, EGG_HULL_MIN, EGG_HULL_MAX);

	t = GetTeam();
	if (!t)
		return;

	noclip_ents.Purge();

	if (spawnpoint) {
		t->AddSpawnpoint(spawnpoint);
		spawnpoint->SetAbsOrigin(GetAbsOrigin());
		spawnpoint->SetAbsAngles(GetAbsAngles());
		spawnpoint->SetParent(this);
		spawnpoint->SetCycleEfficiency(0.35f);
	}

	SetSequence(LookupSequence("idle"));
	SetPlaybackRate(1.0f);
	UseClientSideAnimation();
}

void CEggEntity::InputToggle(inputdata_t &input) {
	if (!active) {
		active = true;
	} else {
		active = false;
	}
}

void CEggEntity::Event_Killed(const CTakeDamageInfo &info) {
	CTeam *t;
	CSoundParameters params;

	t = GetGlobalTeam(TEAM_SPIDERS);
	if (!t)
		return;

	if (spawnpoint) {
		t->RemoveSpawnpoint(spawnpoint);
	}
	
	// BaseClass will reclaim points
	BaseClass::Event_Killed(info);
}

void CEggEntity::SpawnSound(void) {
	CSoundParameters params;

	if (GetParametersForSound(EGG_SPAWN_PLAYER_SOUND, params, NULL) == false)
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

void CEggEntity::EntNoClip(CBaseEntity *e) {
	EHANDLE ent;
	ent = e;

	if (noclip_ents.Find(ent) == noclip_ents.InvalidIndex()) {
		noclip_ents.AddToTail(ent);
	}

	if (noclip_ents.Count() == 1) {
		SetThink(&CEggEntity::NoClipThink);
		SetNextThink(gpGlobals->curtime + NOCLIP_THINK_TIME);
	}
}

//
// make sure the egg doesn't collide with players it just spawned
//
bool CEggEntity::ShouldCollideWithEntity(const CBaseEntity *e) const {
	EHANDLE ent;

	if (e->GetCollisionGroup() == COLLISION_GROUP_WEAPON)
		return false;

	ent = e;

	if (noclip_ents.Find(ent) == noclip_ents.InvalidIndex())
		return true;

	return false;
}

//
// remove noclip'ed entities when they leave the specified radius
// so that egg becomes solid to them
//
void CEggEntity::NoClipThink(void) {
	EHANDLE noclipped;
	EHANDLE found;
	CBaseEntity *e;
	bool keep;
	int i;


	//
	// loop over all noclipped entities
	// if any of them are not found by the sphere query
	// then remove them from the noclip list
	//
	for (i = 0; i < noclip_ents.Count(); i++) {
		noclipped = noclip_ents[i];
		keep = false;

		for (CEntitySphereQuery sphere(GetAbsOrigin(), NOCLIP_RADIUS);
			(e = sphere.GetCurrentEntity()) != NULL;
			sphere.NextEntity()) {
		
			found = e;
			
			if (found == noclipped) {
				keep = true;
				break;
			}
		}

		if (keep == false) {
			noclip_ents.Remove(i);
			i--; // vector shifts, re-assess current position on next loop
		}
	}

	if (noclip_ents.Count() == 0) {
		SetThink(NULL);
	} else {
		SetNextThink(gpGlobals->curtime + NOCLIP_THINK_TIME);
	}
}

CTeamSpawnPoint *CEggEntity::SpawnPoint(void) {
	return spawnpoint;
}

#endif //ifndef CLIENT_DLL