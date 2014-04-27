#include "cbase.h"
#include "teleporter.h"
#include "item_info.h"

LINK_ENTITY_TO_CLASS (ent_teleporter, CTeleporterEntity );
IMPLEMENT_NETWORKCLASS_ALIASED( TeleporterEntity, DT_TeleporterEntity );

BEGIN_NETWORK_TABLE( CTeleporterEntity, DT_TeleporterEntity )
END_NETWORK_TABLE()



CTeleporterEntity::CTeleporterEntity() : CHumanMateriel(&dk_items[ITEM_TELEPORTER_IDX]) {
	active = false;
	autoKill = false;

#ifndef CLIENT_DLL
	spawnpoint = (CTeamSpawnPoint *)CreateEntityByName("info_player_teamspawn");
#else
	sparks = NULL;
#endif
}

CTeleporterEntity::~CTeleporterEntity() {
#ifndef CLIENT_DLL
	UTIL_Remove(spawnpoint);
#endif
}

void CTeleporterEntity::AutoKill(void) {
	autoKill = true;
}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CTeleporterEntity)
END_DATADESC()

void CTeleporterEntity::Precache(void) {
	PrecacheScriptSound(TELEPORTER_SPAWN_PLAYER_SOUND);
	PrecacheParticleSystem(TELEPORTER_SPRITE);
	BaseClass::Precache();
}

//
// Teleporter must start active so that
// those placed in the map can work
// but the default behavior for Engineer-spawned
// teleporters is that they're inactive, so
// have weapon_engy call Disable() instead of
// modifying the more common default behavior
//
void CTeleporterEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	VPhysicsDestroyObject();
	(void)VPhysicsInitStatic();

	SetHealth(item_info->max_health);

	spawnpoint->SetAbsOrigin(GetAbsOrigin());
	spawnpoint->SetAbsAngles(GetAbsAngles());
	spawnpoint->SetParent(this);
}

//
// Teleporters can't be touching other ents (don't want people to spawn in them)
// so kill anything touching the teleporter
//
void CTeleporterEntity::StartTouch(CBaseEntity *e) {
	CMateriel *mat;
	CTeleporterEntity *t;
	
	mat = dynamic_cast<CMateriel *>(e);
	if (mat) {
		t = dynamic_cast<CTeleporterEntity *>(mat);
		if (t) {
			// the bottom-most teleporter wins
			if (t->GetAbsOrigin().z < GetAbsOrigin().z)
				return;
		}

		CTakeDamageInfo info(NULL, NULL, 999, DMG_BLAST);
		info.SetDamagePosition(GetAbsOrigin());
		info.SetDamageForce(Vector(0,0,100));
		mat->TakeDamage(info);
		ApplyMultiDamage();
	}
}

void CTeleporterEntity::InputToggle(inputdata_t &input) {
	if (!active) {
		Warning("Teleporter activating!\n");
		active = true;
	} else {
		active = false;
	}
}

int CTeleporterEntity::TakeHealth(int amt, int type) {
	int ret;
	int health;

	if (autoKill == false) {
		ret = BaseClass::TakeHealth(amt, type);
		health = GetHealth();

		spawnpoint->SetCycleEfficiency(((float)health)/((float)item_info->max_health));

		return ret;
	}

	return 0;
}

int CTeleporterEntity::OnTakeDamage(const CTakeDamageInfo &info) {
	int ret;
	int health;
	
	ret = BaseClass::OnTakeDamage(info);

	health = GetHealth();

	spawnpoint->SetCycleEfficiency(((float)health)/((float)item_info->max_health));

	return ret;
}

void CTeleporterEntity::Event_Killed(const CTakeDamageInfo &info) {
	BaseClass::Event_Killed(info);
}

CTeamSpawnPoint *CTeleporterEntity::SpawnPoint(void) {
	return spawnpoint;
}

void CTeleporterEntity::SpawnSound(void) {
	CSoundParameters params;

	if ( GetParametersForSound( TELEPORTER_SPAWN_PLAYER_SOUND, params, NULL ) == false )
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

void CTeleporterEntity::EnableEntity(void) {
	if (active == false) {
		GetTeam()->AddSpawnpoint(SpawnPoint());
		BaseClass::EnableEntity();
	}
}

void CTeleporterEntity::DisableEntity(void) {
	if (active == true) {
		GetTeam()->RemoveSpawnpoint(SpawnPoint());
		BaseClass::DisableEntity();
	}
}

#else

void CTeleporterEntity::EnableEntity(void) {
	if (sparks == NULL) {
		sparks = ParticleProp()->Create(TELEPORTER_SPRITE, PATTACH_ABSORIGIN_FOLLOW);
	}
}

void CTeleporterEntity::DisableEntity(void) {
	ParticleProp()->StopParticlesInvolving(this);
	sparks = NULL;
}

#endif