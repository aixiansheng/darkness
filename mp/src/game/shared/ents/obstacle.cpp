#include "cbase.h"
#include "obstacle.h"
#include "item_info.h"
#include "class_info.h"
#include "ammodef.h"

LINK_ENTITY_TO_CLASS(ent_obstacle, CObstacleEntity);
IMPLEMENT_NETWORKCLASS_ALIASED(ObstacleEntity, DT_ObstacleEntity);

BEGIN_NETWORK_TABLE(CObstacleEntity, DT_ObstacleEntity)
END_NETWORK_TABLE()

#define MIN_FUNCTIONAL_HEALTH	150

CObstacleEntity::CObstacleEntity() : CSpiderMateriel(&dk_items[ITEM_OBSTACLE_IDX]) {
}

CObstacleEntity::~CObstacleEntity() {
#ifndef CLIENT_DLL
	touching.Purge();
#endif
}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CObstacleEntity)
END_DATADESC()

void CObstacleEntity::Precache(void) {
	BaseClass::Precache();
}

void CObstacleEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();
	SetTouch(&CObstacleEntity::ObsTouch);
	
	//AddSolidFlags(FSOLID_TRIGGER);
	touching.Purge();
	m_flNextTouch = 0.0f;
}

void CObstacleEntity::ObsTouch(CBaseEntity *e) {
	trace_t tr;
	CHL2MP_Player *p;
	EHANDLE ent;
	Vector dir;
	Vector endpos;

	p = dynamic_cast<CHL2MP_Player *>(e);

	if (p) {

		if (m_flNextTouch < gpGlobals->curtime) {
			m_flNextTouch = gpGlobals->curtime + OBSTACLE_TOUCH_INT;
			endpos = p->GetAbsOrigin();
			dir = endpos - GetAbsOrigin();

			CTakeDamageInfo info(this, this, OBSTACLE_DMG_VALUE, DMG_GENERIC|DMG_ALWAYSGIB);
			
			CalculateMeleeDamageForce(&info, dir, endpos, 0.01f);
			info.SetAmmoType(GetAmmoDef()->Index("obstacle"));

			p->DispatchTraceAttack(info, Vector(0,0,1), &tr);
			ApplyMultiDamage();
		}
	}

}

#endif