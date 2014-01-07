#include "cbase.h"
#include "exp_mine.h"


#define MINE_SOLID_THINK_INT 0.2f
#define MINE_SOLID_CTX "mineSolid"

#define MINE_BEAM_SPRITE "sprites/laserbeam.vmt"
#define MINE_BEAM_WIDTH 2.5f

LINK_ENTITY_TO_CLASS(ent_mine, CExpMineEntity);
IMPLEMENT_NETWORKCLASS_ALIASED( ExpMineEntity, DT_ExpMineEntity );

BEGIN_NETWORK_TABLE( CExpMineEntity, DT_ExpMineEntity )
END_NETWORK_TABLE()

CExpMineEntity::CExpMineEntity() : CHumanMateriel(&human_items[ITEM_MINE_IDX]) {
#ifndef CLIENT_DLL
	surfaceNorm = vec3_origin;
	laser = NULL;
#endif
}
CExpMineEntity::~CExpMineEntity() {
}

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

	surfaceNorm = vec3_origin;
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

void CExpMineEntity::SetNormal(Vector &normal) {
	surfaceNorm = normal;
}

void CExpMineEntity::DetectThink(void) {
	trace_t tr;

	SetNextThink(gpGlobals->curtime + EXPMINE_THINK_INTERVAL);
	
	UTIL_TraceLine(GetAbsOrigin(), endpos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	
	if (tr.DidHit() && 
		tr.endpos != endpos && 
		tr.m_pEnt && 
		tr.m_pEnt->IsPlayer() &&
		tr.m_pEnt->GetTeamNumber() == TEAM_SPIDERS &&
		tr.m_pEnt->IsAlive()) {

		ExplosionCreate
		(
			GetAbsOrigin(), 
			GetAbsAngles(), 
			this, 
			EXPMINE_DAMAGE, 
			EXPMINE_RADIUS,
			SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 
			0.0f, 
			NULL
		);

		UTIL_Remove(this);
	}
}

void CExpMineEntity::SetupThink(void) {
	trace_t tr;

	if (surfaceNorm == vec3_origin) {
		UTIL_Remove(this);
	}

	endpos = GetAbsOrigin() + (surfaceNorm * 4096);

	UTIL_TraceLine(GetAbsOrigin(), endpos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	laser = CBeam::BeamCreate( MINE_BEAM_SPRITE, MINE_BEAM_WIDTH );
	if (laser == NULL) {
		UTIL_Remove(this);
	}

	if (tr.DidHit()) {
		endpos = tr.endpos;
	}

	laser->PointEntInit( endpos, this );
	laser->SetBrightness( 130 );
	laser->SetRenderColor( 255, 0, 0, 90 );
	laser->RelinkBeam();

	SetThink(&CExpMineEntity::DetectThink);
	SetNextThink(gpGlobals->curtime + EXPMINE_THINK_INTERVAL);
}

#endif