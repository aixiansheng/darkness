//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Team spawnpoint handling
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "entitylist.h"
#include "entityoutput.h"
#include "player.h"
#include "eventqueue.h"
#include "gamerules.h"
#include "team_spawnpoint.h"
#include "team.h"
#include "hl2mp_player.h"
#include "ents/materiel.h"
#include "bspflags.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


class CTraceFilterOnlyOne : public CTraceFilterSimple {
public:
	DECLARE_CLASS( CTraceFilterOnlyOne, CTraceFilterSimple );
	
	CTraceFilterOnlyOne( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimple( NULL, collisionGroup ), m_pPassNotOwner(passentity)
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask ) {
		Warning("ShouldHitEntity\n");
		if (pHandleEntity == m_pPassNotOwner) {
			Warning("Match!\n");
			return true;
		}

		Warning("Probably not\n");
		return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );
	}

protected:
	const IHandleEntity *m_pPassNotOwner;
};


LINK_ENTITY_TO_CLASS( info_player_teamspawn, CTeamSpawnPoint );

BEGIN_DATADESC( CTeamSpawnPoint )

	// keys
	DEFINE_KEYFIELD( m_iDisabled, FIELD_INTEGER, "StartDisabled" ),

	// input functions
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	// outputs
	DEFINE_OUTPUT( m_OnPlayerSpawn, "OnPlayerSpawn" ),

END_DATADESC()

CTeamSpawnPoint::CTeamSpawnPoint(void) {
	cycleTime = DEFAULT_SPAWN_CYCLE;
	nextSpawnTime = gpGlobals->curtime + DEFAULT_SPAWN_CYCLE;
}

void CTeamSpawnPoint::SetCycleEfficiency(float f) {
	cycleTime = f * DEFAULT_SPAWN_CYCLE;
	nextSpawnTime = gpGlobals->curtime + cycleTime;
}

//-----------------------------------------------------------------------------
// Purpose: Attach this spawnpoint to it's team
//-----------------------------------------------------------------------------
void CTeamSpawnPoint::Activate( void )
{
	nextSpawnTime = gpGlobals->curtime + cycleTime;
	UTIL_DropToFloor(this, MASK_SOLID);
	BaseClass::Activate();
}

void CTeamSpawnPoint::Spawn(void) {
	nextSpawnTime = gpGlobals->curtime + cycleTime;
	spawns = 0;
}

void CTeamSpawnPoint::SpawnPlayer(CBasePlayer *p) {
	CMateriel *mat;
	CHL2MP_Player *creator;
	CHL2MP_Player *spawnee;
	CBaseEntity *ent;
	Vector spawnOrigin;

	nextSpawnTime = gpGlobals->curtime + cycleTime;

	spawnOrigin = GetAbsOrigin() + Vector(0,0,40);

	spawnee = ToHL2MPPlayer(p);
	if (!spawnee)
		return;

	//
	// telefrag anyone who would collide with the player
	//

	for (CEntitySphereQuery sphere(spawnOrigin, 100); 
		(ent = sphere.GetCurrentEntity()) != NULL; 
		sphere.NextEntity()) 
	{
		if (ent->IsPlayer()) {
			trace_t tr;
			CTraceFilterOnlyOne filter(ent, COLLISION_GROUP_NONE);

			UTIL_TraceHull(spawnOrigin, spawnOrigin, spawnee->classVectors->m_vHullMin, spawnee->classVectors->m_vHullMax, MASK_SHOT, &filter, &tr);
			
			if (tr.DidHit()) {
				// telefrag 'em
				trace_t tr2;
				CTakeDamageInfo info(this, this, 99999.0f, DMG_BLAST|DMG_ALWAYSGIB);
				CalculateMeleeDamageForce(&info, Vector(0,0,1), GetAbsOrigin(), 0.02f);
				ent->DispatchTraceAttack(info, Vector(0,0,1), &tr2);
				ApplyMultiDamage();
			}
		}
	}


	//
	// set player position... (from gamerules.cpp)
	//

	p->SetLocalOrigin( spawnOrigin );
	p->SetAbsVelocity( vec3_origin );
	p->SetLocalAngles( GetLocalAngles() );
	p->m_Local.m_vecPunchAngle = vec3_angle;
	p->m_Local.m_vecPunchAngleVel = vec3_angle;
	p->SnapEyeAngles( GetLocalAngles() );

	ToHL2MPPlayer(p)->Spawn();
	ToHL2MPPlayer(p)->NoClipSpawn(this->GetParent());

	spawns++;

	mat = dynamic_cast<CMateriel *>(this->GetParent());
	creator = mat->GetCreator();
	if (creator != NULL && creator->GetTeamNumber() == p->GetTeamNumber()) {
		if (spawns % 5 == 0) {
			creator->IncrementFragCount(1);
		}
	}
}

bool CTeamSpawnPoint::IsValid( CBasePlayer *pPlayer ) {
	if (nextSpawnTime < gpGlobals->curtime)
		return true;
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeamSpawnPoint::InputEnable( inputdata_t &inputdata )
{
	m_iDisabled = FALSE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeamSpawnPoint::InputDisable( inputdata_t &inputdata )
{
	m_iDisabled = TRUE;
}

//===========================================================================================================
// VEHICLE SPAWNPOINTS
//===========================================================================================================
LINK_ENTITY_TO_CLASS( info_vehicle_groundspawn, CTeamVehicleSpawnPoint );

BEGIN_DATADESC( CTeamVehicleSpawnPoint )

	// outputs
	DEFINE_OUTPUT( m_OnVehicleSpawn, "OnVehicleSpawn" ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Is this spawnpoint ready for a vehicle to spawn in?
//-----------------------------------------------------------------------------
bool CTeamVehicleSpawnPoint::IsValid( void )
{
	CBaseEntity *ent = NULL;
	for ( CEntitySphereQuery sphere( GetAbsOrigin(), 128 ); ( ent = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		// if ent is a client, don't spawn on 'em
		CBaseEntity *plent = ent;
		if ( plent && plent->IsPlayer() )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Attach this spawnpoint to it's team
//-----------------------------------------------------------------------------
void CTeamVehicleSpawnPoint::Activate( void )
{
	BaseClass::Activate();
	if ( GetTeamNumber() > 0 && GetTeamNumber() <= MAX_TEAMS )
	{
		// Don't add vehicle spawnpoints to the team for now
		//GetGlobalTeam( GetTeamNumber() )->AddSpawnpoint( this );
	}
	else
	{
		Warning( "info_vehicle_groundspawn with invalid team number: %d\n", GetTeamNumber() );
		UTIL_Remove( this );
	}
}
