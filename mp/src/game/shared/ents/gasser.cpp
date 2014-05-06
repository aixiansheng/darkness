#include "cbase.h"
#include "gasser.h"
#include "vstdlib/random.h"

#ifndef CLIENT_DLL
#include "grenade_acid.h"
#include "basegrenade_shared.h"
#endif

#define GASSER_DPS 15

LINK_ENTITY_TO_CLASS(ent_gasser, CGasserEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( GasserEntity, DT_GasserEntity );

BEGIN_NETWORK_TABLE( CGasserEntity, DT_GasserEntity )
END_NETWORK_TABLE()

CGasserEntity::CGasserEntity() : CSpiderMateriel(&dk_items[ITEM_GASSER_IDX]) {}
CGasserEntity::~CGasserEntity() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CGasserEntity)
	DEFINE_THINKFUNC(RechargeThink),
END_DATADESC()


void CGasserEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	SetSolidFlags(FSOLID_NOT_STANDABLE);
	UTIL_SetSize(this, GASSER_HULL_MIN, GASSER_HULL_MAX);

	SetThink(&CGasserEntity::RechargeThink);
	SetNextThink(gpGlobals->curtime + GASSER_RECHARGE_TIME);
}

// fire some gas, recharge
void CGasserEntity::RechargeThink(void) {
	Vector v;
	QAngle a;
	CBaseGrenade *gren;

	a = GetAbsAngles();
	AngleVectors(a, &v);

	v.x += random->RandomFloat(-0.1f, 0.1f);
	v.y += random->RandomFloat(0.0f, 0.2f);
	v.z += 10.0f;

	SetNextThink(gpGlobals->curtime + GASSER_RECHARGE_TIME);

	gren = AcidGren_Create
	(
		GetAbsOrigin(),
		a,
		v * 30,
		vec3_origin,
		this,
		2.0f,
		false
	);

	if (gren) {
		gren->SetDamage(GASSER_DPS);
	}
}


#endif
