#include "cbase.h"
#include "medipad.h"
#include "item_info.h"

LINK_ENTITY_TO_CLASS(ent_medipad, CMedipadEntity);
IMPLEMENT_NETWORKCLASS_ALIASED(MedipadEntity, DT_MedipadEntity);

BEGIN_NETWORK_TABLE( CMedipadEntity, DT_MedipadEntity )
END_NETWORK_TABLE()

#define MIN_FUNCTIONAL_HEALTH	150

CMedipadEntity::CMedipadEntity() : CHumanMateriel(&human_items[ITEM_MEDIPAD_IDX]) {
#ifndef CLIENT_DLL
	active = true;
#endif
}

CMedipadEntity::~CMedipadEntity() {
#ifndef CLIENT_DLL
	touching.Purge();
#endif
}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CMedipadEntity)
	DEFINE_FIELD(active, FIELD_BOOLEAN),
	DEFINE_THINKFUNC(HealThink),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
END_DATADESC()

//bool CMedipadEntity::CreateVPhysics( void ) {
//	return ( VPhysicsInitStatic() != NULL );
//}

void CMedipadEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	//SetSolid(SOLID_VPHYSICS);
	VPhysicsDestroyObject();
	(void)VPhysicsInitStatic();

	m_nSkin = 1;
	touching.Purge();
	active = true;
	healed_total = 0;
}

void CMedipadEntity::InputToggle(inputdata_t &input) {
	if (!active) {
		active = true;
	} else {
		active = false;
	}
}

void CMedipadEntity::HealThink(void) {
	int i;
	int before, after;
	trace_t tr;
	CHL2MP_Player *p;
	EHANDLE ent;

	SetNextThink(gpGlobals->curtime + MEDIPAD_THINK_INTERVAL);
	
	for (i = 0; i < touching.Count(); i++) {
		ent = touching[i];
		p = dynamic_cast<CHL2MP_Player *>(ent.Get());

		if (!ent || !p) {

			touching.Remove(i);

		} else {
	
			if (active) {
				before = p->GetHealth();
				p->TakeHealth(MEDIPAD_HEAL_VALUE, DMG_GENERIC);
				after = p->GetHealth();

				if (after - before > 0) {
					healed_total += (after - before);

					//
					// give creator frags for maintaining a useful medipad every 100 pts of healing
					//

					if (healed_total % 100 == 0 && GetCreator() && GetCreator()->GetTeamNumber() == GetTeamNumber()) {
						GetCreator()->IncrementFragCount(1);
					}
				}
			}

		}
	}

	if (touching.Count() == 0) {
		SetThink(NULL);
	}
}

void CMedipadEntity::StartTouch(CBaseEntity *e) {
	CHL2MP_Player *p;
	EHANDLE ent;
	ent = e;

	if ((p = dynamic_cast<CHL2MP_Player *>(e)) != NULL && p->GetTeamNumber() == GetTeamNumber()) {
		if (touching.Find(ent) == touching.InvalidIndex()) {
			touching.AddToTail(ent);
		}

		if (touching.Count() == 1) {
			SetThink(&CMedipadEntity::HealThink);
			SetNextThink(MEDIPAD_THINK_INTERVAL);
		}
	}

	BaseClass::StartTouch(e);
}

void CMedipadEntity::EndTouch(CBaseEntity *e) {
	EHANDLE ent;
	ent = e;

	touching.FindAndRemove(ent);
	if (touching.Count() == 0) {
		SetThink(NULL);
	}

	BaseClass::EndTouch(e);
}

#endif