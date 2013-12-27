#include "cbase.h"
#include "exp_mine.h"


#define MINE_SOLID_THINK_INT 0.2f
#define MINE_SOLID_CTX "mineSolid"



LINK_ENTITY_TO_CLASS(ent_mine, CExpMineEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( ExpMineEntity, DT_ExpMineEntity );

BEGIN_NETWORK_TABLE( CExpMineEntity, DT_ExpMineEntity )
END_NETWORK_TABLE()

CExpMineEntity::CExpMineEntity() : CHumanMateriel(&human_items[ITEM_MINE_IDX]) {}
CExpMineEntity::~CExpMineEntity() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CExpMineEntity)
	DEFINE_THINKFUNC(DetectThink),
	DEFINE_THINKFUNC(SetupThink),
	DEFINE_THINKFUNC(SolidThink),
END_DATADESC()

void CExpMineEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	SetCollisionGroup(COLLISION_GROUP_DEBRIS);

	SetThink(&CExpMineEntity::SetupThink);
	SetNextThink(gpGlobals->curtime + EXPMINE_ARM_TIME);

	RegisterThinkContext(MINE_SOLID_CTX);
	SetContextThink(&CExpMineEntity::SolidThink, gpGlobals->curtime + MINE_SOLID_THINK_INT, MINE_SOLID_CTX);
}

void CExpMineEntity::SolidThink(void) {
	CBaseEntity *ent;

	for (CEntitySphereQuery sphere(GetAbsOrigin(), 5); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		if (ent->IsPlayer()) {
			SetContextThink(&CExpMineEntity::SolidThink, gpGlobals->curtime + MINE_SOLID_THINK_INT, MINE_SOLID_CTX);
			return;
		}
	}

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
}

void CExpMineEntity::DetectThink(void) {
	CBaseEntity *ent;
	trace_t tr;

	SetNextThink(gpGlobals->curtime + EXPMINE_THINK_INTERVAL);
	
	for (CEntitySphereQuery sphere(GetAbsOrigin(), EXPMINE_RADIUS / 2); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		UTIL_TraceLine(GetAbsOrigin(), ent->GetAbsOrigin(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		
		if (tr.DidHit() && tr.m_pEnt != ent)
			continue; // something's in the way

		if (ent->IsPlayer() && ent->GetTeamNumber() == TEAM_SPIDERS && ent->IsAlive()) {
			ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), this, EXPMINE_DAMAGE, EXPMINE_RADIUS,
				SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, NULL);
			UTIL_Remove(this);
		}
	}
}

void CExpMineEntity::SetupThink(void) {
	// make this client side..
	// Warning("Mine Armed\n");
	SetThink(&CExpMineEntity::DetectThink);
	SetNextThink(gpGlobals->curtime + EXPMINE_THINK_INTERVAL);
}

#endif