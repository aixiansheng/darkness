#include "cbase.h"
#include "class_info.h"
#include "corpse_parent.h"

#define CORPSE_PARENT_MODEL "models/infested.mdl"

CCorpseParent::CCorpseParent() : CBaseAnimating() {
}

CCorpseParent::~CCorpseParent(void) {
}

LINK_ENTITY_TO_CLASS(corpse_parent, CCorpseParent);
IMPLEMENT_NETWORKCLASS_ALIASED( CorpseParent, DT_CorpseParent )

BEGIN_NETWORK_TABLE( CCorpseParent, DT_CorpseParent )
END_NETWORK_TABLE()


#ifndef CLIENT_DLL

BEGIN_DATADESC(CCorpseParent)
END_DATADESC()

void CCorpseParent::Spawn(void) {
	BaseClass::Precache();
	BaseClass::Spawn();

	//SetModel(CORPSE_PARENT_MODEL);
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_FLYGRAVITY);
	AddSolidFlags(FSOLID_NOT_SOLID);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	m_takedamage = DAMAGE_NO;
}

#endif
