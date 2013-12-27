#include "cbase.h"
#include "obstacle.h"
#include "item_info.h"
#include "class_info.h"

LINK_ENTITY_TO_CLASS(ent_obstacle, CObstacleEntity);
IMPLEMENT_NETWORKCLASS_ALIASED(ObstacleEntity, DT_ObstacleEntity);

BEGIN_NETWORK_TABLE(CObstacleEntity, DT_ObstacleEntity)
END_NETWORK_TABLE()

#define MIN_FUNCTIONAL_HEALTH	150

CObstacleEntity::CObstacleEntity() : CSpiderMateriel(&spider_items[ITEM_OBSTACLE_IDX]) {
}

CObstacleEntity::~CObstacleEntity() {
#ifndef CLIENT_DLL
	touching.Purge();
#endif
}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CObstacleEntity)
	DEFINE_THINKFUNC(DmgThink),
END_DATADESC()

void CObstacleEntity::Precache(void) {
	BaseClass::Precache();
}

void CObstacleEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();
	
	AddSolidFlags(FSOLID_TRIGGER);

	touching.Purge();
}

void CObstacleEntity::DmgThink(void) {
	int i;
	trace_t tr;
	CHL2MP_Player *p;
	EHANDLE ent;
	Vector dir;
	Vector endpos;

	SetNextThink(gpGlobals->curtime + OBSTACLE_ACTIVE_THINK_INVERVAL);
	
	for (i = 0; i < touching.Count(); i++) {
		ent = touching[i];
		p = dynamic_cast<CHL2MP_Player *>(ent.Get());

		if (!ent || !p || p->GetTeamNumber() == GetTeamNumber()) {
			touching.Remove(i);
		} else {
			endpos = p->GetAbsOrigin();
			dir = endpos - GetAbsOrigin();

			CTakeDamageInfo info(this, this, OBSTACLE_DMG_VALUE, DMG_GENERIC|DMG_ALWAYSGIB);
			CalculateMeleeDamageForce(&info, dir, endpos, 0.01f);
			p->DispatchTraceAttack(info, Vector(0,0,1), &tr);
			ApplyMultiDamage();

		}
	}

	if (touching.Count() == 0) {
		SetThink(NULL);
	}
}

void CObstacleEntity::StartTouch(CBaseEntity *e) {
	CHL2MP_Player *p;
	EHANDLE ent;
	ent = e;

	if ((p = dynamic_cast<CHL2MP_Player *>(e)) != NULL && p->GetTeamNumber() != GetTeamNumber()) {
		if (touching.Find(ent) == touching.InvalidIndex()) {
			touching.AddToTail(ent);
		}

		if (touching.Count() == 1) {
			SetThink(&CObstacleEntity::DmgThink);
			SetNextThink(OBSTACLE_ACTIVE_THINK_INVERVAL);
		}
	}

	BaseClass::StartTouch(e);
}

void CObstacleEntity::EndTouch(CBaseEntity *e) {
	EHANDLE ent;
	ent = e;

	touching.FindAndRemove(ent);
	if (touching.Count() == 0) {
		SetThink(NULL);
	}

	BaseClass::EndTouch(e);
}

#endif