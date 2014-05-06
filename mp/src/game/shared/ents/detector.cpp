#include "cbase.h"
#include "detector.h"

#ifndef CLIENT_DLL
#include "hl2mp_player.h"
#endif


#define DETECTOR_SOLID_CTX "solidThink"
#define DETECTOR_SOLID_THINK_INT 0.2f


LINK_ENTITY_TO_CLASS(ent_detector, CDetectorEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( DetectorEntity, DT_DetectorEntity );

BEGIN_NETWORK_TABLE( CDetectorEntity, DT_DetectorEntity )
END_NETWORK_TABLE()

CDetectorEntity::CDetectorEntity() : CHumanMateriel(&dk_items[ITEM_DETECTOR_IDX]) {}
CDetectorEntity::~CDetectorEntity() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CDetectorEntity)
	DEFINE_THINKFUNC(DetectThink),
	DEFINE_THINKFUNC(SetupThink),
	DEFINE_THINKFUNC(SolidThink),
END_DATADESC()

void CDetectorEntity::Precache(void) {
	PrecacheScriptSound(DETECTOR_SOUND);
	BaseClass::Precache();
}

void CDetectorEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	//SetSolid(SOLID_BSP);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);

	SetThink(&CDetectorEntity::SetupThink);
	SetNextThink(gpGlobals->curtime + DETECTOR_ARM_TIME);

	RegisterThinkContext(DETECTOR_SOLID_CTX);
	SetContextThink
	(
		&CDetectorEntity::SolidThink,
		gpGlobals->curtime + DETECTOR_SOLID_THINK_INT,
		DETECTOR_SOLID_CTX
	);
}

void CDetectorEntity::SolidThink(void) {
	CBaseEntity *ent;

	for (CEntitySphereQuery sphere(GetAbsOrigin(), 5); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		if (ent->IsPlayer()) {
			SetContextThink
			(
				&CDetectorEntity::SolidThink,
				gpGlobals->curtime + DETECTOR_SOLID_THINK_INT,
				DETECTOR_SOLID_CTX
			);

			return;
		}
	}

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
}

void CDetectorEntity::DetectThink(void) {
	CBaseEntity *ent;
	CHL2MP_Player *p;
	trace_t tr;
	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;
	Vector origin;

	SetNextThink(gpGlobals->curtime + DETECTOR_THINK_INTERVAL);
	
	for (CEntitySphereQuery sphere(GetAbsOrigin(), DETECTOR_RADIUS); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		if (ent->IsPlayer() && ent->GetTeamNumber() == TEAM_SPIDERS && ent->IsAlive()) {
			if (GetParametersForSound(DETECTOR_SOUND, params, NULL)) {

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

			p = ToHL2MPPlayer(ent);
			if (p) {
				p->Detected();
			}
		}
	}
}

void CDetectorEntity::SetupThink(void) {
	// make this client side..
	// Warning("Mine Armed\n");
	SetThink(&CDetectorEntity::DetectThink);
	SetNextThink(gpGlobals->curtime + DETECTOR_THINK_INTERVAL);
}

#endif
