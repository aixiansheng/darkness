#include "cbase.h"
#include "ents/healer.h"
#include "item_info.h"

LINK_ENTITY_TO_CLASS(ent_healer, CHealerEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( HealerEntity, DT_HealerEntity );

BEGIN_NETWORK_TABLE(CHealerEntity, DT_HealerEntity)
END_NETWORK_TABLE()

CHealerEntity::CHealerEntity() : CSpiderMateriel(&spider_items[ITEM_HEALER_IDX]) {
}

CHealerEntity::~CHealerEntity() {
#ifndef CLIENT_DLL
	touching.Purge();
#endif
}


#ifndef CLIENT_DLL

BEGIN_DATADESC(CHealerEntity)
	DEFINE_THINKFUNC(HealThink),
	DEFINE_THINKFUNC(HealThrottleThink),
END_DATADESC()


void CHealerEntity::Precache(void) {
	BaseClass::Precache();
	PrecacheScriptSound(HEALER_GARGLE_SOUND);
}

void CHealerEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	UTIL_SetSize(this, HEALER_HULL_MIN, HEALER_HULL_MAX);

	touching.Purge();
	next_gargle = FLT_MAX;

	SetSequence(LookupSequence("idle"));
	SetPlaybackRate(1.0f);
	UseClientSideAnimation();

	RegisterThinkContext(HEAL_THROTTLE_CTX);
	SetContextThink(&CHealerEntity::HealThrottleThink, gpGlobals->curtime + HEAL_THROTTLE_INT, HEAL_THROTTLE_CTX);

	healed_total = 0;
}

//
// sphere query, add players to touching list
// then set think if list is not empty
//
void CHealerEntity::HealThrottleThink(void) {
	CBaseEntity *ent;
	CHL2MP_Player *p;
	EHANDLE enth;
	trace_t tr;

	SetNextThink(gpGlobals->curtime + HEAL_THROTTLE_INT, HEAL_THROTTLE_CTX);

	touching.Purge();

	for (CEntitySphereQuery sphere(GetAbsOrigin(), HEAL_RADIUS); 
		(ent = sphere.GetCurrentEntity()) != NULL;
		sphere.NextEntity()) 
	{
		UTIL_TraceLine(GetAbsOrigin(), ent->GetAbsOrigin(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		
		if (tr.DidHit())
			continue; // something's in the way

		// only add players on the same team
		if ((p = dynamic_cast<CHL2MP_Player *>(ent)) != NULL && p->GetTeamNumber() == GetTeamNumber()) {
			enth = ent;

			if (touching.Find(enth) == touching.InvalidIndex()) {
				touching.AddToTail(enth);
			}
		}

	}

	if (touching.Count() > 0) {
		SetThink(&CHealerEntity::HealThink);
		SetNextThink(gpGlobals->curtime);
	} else {
		SetThink(NULL);
	}

}

void CHealerEntity::HealThink(void) {
	int i;
	int before, after;
	trace_t tr;
	CHL2MP_Player *p;
	EHANDLE ent;
	CSoundParameters params;
	EmitSound_t ep;
	Vector vecOrigin;

	SetNextThink(gpGlobals->curtime + HEALER_ACTIVE_THINK_INVERVAL);
	
	for (i = 0; i < touching.Count(); i++) {
		ent = touching[i];
		if (ent) {
			p = dynamic_cast<CHL2MP_Player *>(ent.Get());
			if (p) {
				before = p->GetHealth();
				p->TakeHealth(HEALER_HEAL_VALUE, DMG_GENERIC);
				p->IncrementArmorValue(HEALER_HEAL_VALUE, p->GetMaxArmor());
				after = p->GetHealth();

				if (after - before > 0) {
					healed_total += (after - before);

					//
					// give creator frags for maintaining a useful medipad every 100 pts of healing
					//

					if (healed_total % 75 == 0 && GetCreator() && GetCreator()->GetTeamNumber() == GetTeamNumber()) {
						GetCreator()->IncrementFragCount(1);
					}
				}


			}
		}
	}
	
	if (next_gargle < gpGlobals->curtime) {
		next_gargle = gpGlobals->curtime + HEALER_GARGLE_INTERVAL;

		if (GetParametersForSound( HEALER_GARGLE_SOUND, params, NULL ) == false)
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







