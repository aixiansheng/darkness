#include "cbase.h"
#include "ents/healer.h"
#include "item_info.h"

LINK_ENTITY_TO_CLASS(ent_healer, CHealerEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( HealerEntity, DT_HealerEntity );

BEGIN_NETWORK_TABLE(CHealerEntity, DT_HealerEntity)
END_NETWORK_TABLE()

CHealerEntity::CHealerEntity() : CSpiderMateriel(&dk_items[ITEM_HEALER_IDX]) {
}

CHealerEntity::~CHealerEntity() {
}


#ifndef CLIENT_DLL

BEGIN_DATADESC(CHealerEntity)
	DEFINE_THINKFUNC(HealThink),
END_DATADESC()


void CHealerEntity::Precache(void) {
	BaseClass::Precache();
	PrecacheScriptSound(HEALER_GARGLE_SOUND);
}

void CHealerEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	UTIL_SetSize(this, HEALER_HULL_MIN, HEALER_HULL_MAX);

	next_gargle = FLT_MAX;

	SetSequence(LookupSequence("idle"));
	SetPlaybackRate(1.0f);
	UseClientSideAnimation();

	healed_total = 0;

	SetThink(&CHealerEntity::HealThink);
	SetNextThink(gpGlobals->curtime + HEALER_THINK_INTERVAL);
}

void CHealerEntity::HealThink(void) {
	int before, after;
	int last_inc, this_inc;
	trace_t tr;
	CHL2MP_Player *p;
	CBaseEntity *ent;
	CSoundParameters params;
	EmitSound_t ep;
	Vector vecOrigin;

	SetNextThink(gpGlobals->curtime + HEALER_THINK_INTERVAL);
	
	for (CEntitySphereQuery sphere(GetAbsOrigin(), HEALER_RADIUS);
		(ent = sphere.GetCurrentEntity()) != NULL;
		sphere.NextEntity())
	{
		UTIL_TraceLine
		(
			GetAbsOrigin() + Vector(0,0,10),
			ent->GetAbsOrigin(),
			MASK_SHOT,
			this,
			COLLISION_GROUP_NONE,
			&tr
		);
	
		if (tr.DidHit() && tr.m_pEnt && tr.m_pEnt->edict() != ent->edict())
			continue;

		p = dynamic_cast<CHL2MP_Player *>(ent);
		if (p && p->GetTeamNumber() == TEAM_SPIDERS) {
			if (active) {
				before = p->GetHealth();
				p->TakeHealth(HEALER_HEAL_VALUE, DMG_GENERIC);
				after = p->GetHealth();
				(void)p->EntRefil(HEALER_REFIL_INTERVAL);

				if (after - before > 0) {
					last_inc = healed_total / 300;
					healed_total += (after - before);
					this_inc = healed_total / 300;

					//
					// give creator frags for maintaining a useful medipad every so often
					//
					if (this_inc > last_inc && GetCreator() && GetCreator()->GetTeamNumber() == GetTeamNumber()) {
						GetCreator()->IncrementFragCount(1);
					}
				}

				p->IncrementArmorValue(1, p->GetMaxArmor());
			}
		}
	}

	if (next_gargle < gpGlobals->curtime) {
		next_gargle = gpGlobals->curtime + HEALER_GARGLE_INTERVAL;

		if (GetParametersForSound(HEALER_GARGLE_SOUND, params, NULL ) == false)
			return;

		vecOrigin = GetAbsOrigin();
	
		CRecipientFilter filter;
		filter.AddRecipientsByPAS( vecOrigin );
		
		ep.m_nChannel = params.channel;
		ep.m_pSoundName = params.soundname;
		ep.m_flVolume = params.volume;
		ep.m_SoundLevel = params.soundlevel;
		ep.m_nFlags = 0;
		ep.m_nPitch = params.pitch;
		ep.m_pOrigin = &vecOrigin;

		EmitSound( filter, entindex(), ep );
	}
}

#endif







