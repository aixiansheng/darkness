//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "team.h"
#include "player.h"
#include "team_spawnpoint.h"
#include "gameinterface.h"
#include "ents/teleporter.h"
#include "ents/egg.h"
#include "ents/point_set.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CUtlVector< CTeam * > g_Teams;
CTeam *g_SpiderTeam;
CTeam *g_HumanTeam;
CPointSet *g_PointSet;

//-----------------------------------------------------------------------------
// Purpose: SendProxy that converts the Team's player UtlVector to entindexes
//-----------------------------------------------------------------------------
void SendProxy_PlayerList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CTeam *pTeam = (CTeam*)pData;

	// If this assertion fails, then SendProxyArrayLength_PlayerArray must have failed.
	Assert( iElement < pTeam->m_aPlayers.Count() );

	CBasePlayer *pPlayer = pTeam->m_aPlayers[iElement];
	pOut->m_Int = pPlayer->entindex();
}


int SendProxyArrayLength_PlayerArray( const void *pStruct, int objectID )
{
	CTeam *pTeam = (CTeam*)pStruct;
	return pTeam->m_aPlayers.Count();
}

// Datatable
IMPLEMENT_SERVERCLASS_ST_NOBASE(CTeam, DT_Team)
	SendPropInt( SENDINFO(m_iTeamNum), 5 ),
	SendPropInt( SENDINFO(m_iScore), 0 ),
	SendPropInt( SENDINFO(m_iRoundsWon), 8 ),
	SendPropInt( SENDINFO(asset_points), 10, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(num_spawns), 5, SPROP_UNSIGNED ),
	SendPropString( SENDINFO( m_szTeamname ) ),

	SendPropArray2( 
		SendProxyArrayLength_PlayerArray,
		SendPropInt("player_array_element", 0, 4, 10, SPROP_UNSIGNED, SendProxy_PlayerList), 
		MAX_PLAYERS, 
		0, 
		"player_array"
		)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( team_manager, CTeam );

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified team manager
//-----------------------------------------------------------------------------
CTeam *GetGlobalTeam( int iIndex )
{
	if ( iIndex < 0 || iIndex >= GetNumberOfTeams() )
		return NULL;

	return g_Teams[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: Get the number of team managers
//-----------------------------------------------------------------------------
int GetNumberOfTeams( void )
{
	return g_Teams.Count();
}

bool CTeam::spend_points(int p) {
	if (p > asset_points)
		return false;
	
	asset_points -= p;
	return true;
}

void CTeam::reclaim_points(int p) {
	asset_points += p;
}

//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
CTeam::CTeam( void )
{
	memset( m_szTeamname.GetForModify(), 0, sizeof(m_szTeamname) );
	asset_points = STUB_POINT_VALUE;
	num_spawns = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTeam::~CTeam( void )
{
	spawnQueue.Purge();
	m_aSpawnPoints.Purge();
	m_aPlayers.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame
//-----------------------------------------------------------------------------
void CTeam::Think( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Teams are always transmitted to clients
//-----------------------------------------------------------------------------
int CTeam::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Visibility/scanners
//-----------------------------------------------------------------------------
bool CTeam::ShouldTransmitToPlayer( CBasePlayer* pRecipient, CBaseEntity* pEntity )
{
	// Always transmit the observer target to players
	if ( pRecipient && pRecipient->IsObserver() && pRecipient->GetObserverTarget() == pEntity )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
void CTeam::Init( const char *pName, int iNumber )
{
	m_iScore = 0;

	Q_strncpy( m_szTeamname.GetForModify(), pName, MAX_TEAM_NAME_LENGTH );
	m_iTeamNum = iNumber;

	InitializeSpawnpoints();
	InitializePlayers();

	asset_points = STUB_POINT_VALUE;

	num_spawns = 0;
}

void CTeam::InitPoints(void) {
	int delta;

	//
	// this is the number of points used in level init
	// which should be subtracted from the CPointSet's
	// points
	//
	delta = STUB_POINT_VALUE - asset_points;

	if (m_iTeamNum == TEAM_SPIDERS) {
		asset_points = g_PointSet->SpiderPoints() - delta;
	} else if (m_iTeamNum == TEAM_HUMANS) {
		asset_points = g_PointSet->HumanPoints() - delta;
	}

	if (asset_points < 0) {
		asset_points = 0;
	}
}

void CTeam::RespawnPlayer(void) {
	int i;
	int cnt;
	int r;
	EHANDLE ent;
	CBasePlayer *p;
	CTeamSpawnPoint *sp;
	CBaseEntity *parent;
	CEggEntity *egg = NULL;
	CTeleporterEntity *tele = NULL;

	if (m_aSpawnPoints.Count() == 0)
		return;

	if (spawnQueue.Count() == 0)
		return;

	//ent = spawnQueue.Head();
	ent = spawnQueue[0];
	p = dynamic_cast<CBasePlayer *>(ent.Get());
	if (p == NULL)
		return;

	//
	// try to find a spawn for the player
	// and dequeue the player if one is found
	//
	cnt = m_aSpawnPoints.Count();
	r = random->RandomInt(1, cnt);
	for (i = r; i < r + cnt; i++) {
		sp = m_aSpawnPoints[i % cnt];
		if (sp->IsValid(p)) {
			parent = sp->GetParent();
			if (parent) {
				if ((egg = dynamic_cast<CEggEntity *>(parent)) != NULL) {
					egg->SpawnSound();
				} else if ((tele = dynamic_cast<CTeleporterEntity *>(parent)) != NULL) {
					tele->SpawnSound();
				}
			}

			sp->SpawnPlayer(p);
			spawnQueue.Remove(0);

			// move spawn to end of list
			break;
		}
	}
}

void CTeam::EnqueueSpawn(CBasePlayer *p) {
	EHANDLE ent;

	ent = p;

	if (spawnQueue.Find(ent) == spawnQueue.InvalidIndex()) {
		spawnQueue.AddToTail(ent);
	}
}

//-----------------------------------------------------------------------------
// DATA HANDLING
//-----------------------------------------------------------------------------
int CTeam::GetTeamNumber( void ) const
{
	return m_iTeamNum;
}

//-----------------------------------------------------------------------------
// Purpose: Get the team's name
//-----------------------------------------------------------------------------
const char *CTeam::GetName( void )
{
	return m_szTeamname;
}


//-----------------------------------------------------------------------------
// Purpose: Update the player's client data
//-----------------------------------------------------------------------------
void CTeam::UpdateClientData( CBasePlayer *pPlayer )
{
}

//------------------------------------------------------------------------------------------------------------------
// SPAWNPOINTS
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeam::InitializeSpawnpoints( void )
{
	m_iLastSpawn = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeam::AddSpawnpoint( CTeamSpawnPoint *pSpawnpoint )
{
	m_aSpawnPoints.AddToTail( pSpawnpoint );
	num_spawns = m_aSpawnPoints.Count();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeam::RemoveSpawnpoint( CTeamSpawnPoint *pSpawnpoint )
{
	for (int i = 0; i < m_aSpawnPoints.Count(); i++ )
	{
		if ( m_aSpawnPoints[i] == pSpawnpoint )
		{
			m_aSpawnPoints.Remove( i );
			goto ret;
		}
	}

ret:
	num_spawns = m_aSpawnPoints.Count();
}

bool CTeam::HasSpawnPoints(void) {
	if (m_aSpawnPoints.Count() > 0)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the player at one of this team's spawnpoints. Return true if successful.
//-----------------------------------------------------------------------------
CBaseEntity *CTeam::SpawnPlayer( CBasePlayer *pPlayer )
{
	if ( m_aSpawnPoints.Count() == 0 )
		return NULL;

	// Randomize the start spot
	int iSpawn = m_iLastSpawn + random->RandomInt( 1,3 );
	if ( iSpawn >= m_aSpawnPoints.Count() )
		iSpawn -= m_aSpawnPoints.Count();
	int iStartingSpawn = iSpawn;

	// Now loop through the spawnpoints and pick one
	int loopCount = 0;
	do 
	{
		if ( iSpawn >= m_aSpawnPoints.Count() )
		{
			++loopCount;
			iSpawn = 0;
		}

		// check if pSpot is valid, and that the player is on the right team
		if ( (loopCount > 3) || m_aSpawnPoints[iSpawn]->IsValid( pPlayer ) )
		{
			// DevMsg( 1, "player: spawning at (%s)\n", STRING(m_aSpawnPoints[iSpawn]->m_iName) );
			m_aSpawnPoints[iSpawn]->m_OnPlayerSpawn.FireOutput( pPlayer, m_aSpawnPoints[iSpawn] );
			m_aSpawnPoints[iSpawn]->SpawnPlayer(pPlayer);

			m_iLastSpawn = iSpawn;
			return m_aSpawnPoints[iSpawn];
		}

		iSpawn++;
	} while ( iSpawn != iStartingSpawn ); // loop if we're not back to the start

	return NULL;
}

//------------------------------------------------------------------------------------------------------------------
// PLAYERS
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeam::InitializePlayers( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Add the specified player to this team. Remove them from their current team, if any.
//-----------------------------------------------------------------------------
void CTeam::AddPlayer( CBasePlayer *pPlayer )
{
	m_aPlayers.AddToTail( pPlayer );
	NetworkStateChanged();
}

//-----------------------------------------------------------------------------
// Purpose: Remove this player from the team
//-----------------------------------------------------------------------------
void CTeam::RemovePlayer( CBasePlayer *pPlayer )
{
	m_aPlayers.FindAndRemove( pPlayer );
	NetworkStateChanged();
}

//-----------------------------------------------------------------------------
// Purpose: Return the number of players in this team.
//-----------------------------------------------------------------------------
int CTeam::GetNumPlayers( void )
{
	return m_aPlayers.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Get a specific player
//-----------------------------------------------------------------------------
CBasePlayer *CTeam::GetPlayer( int iIndex )
{
	Assert( iIndex >= 0 && iIndex < m_aPlayers.Count() );
	return m_aPlayers[ iIndex ];
}

//------------------------------------------------------------------------------------------------------------------
// SCORING
//-----------------------------------------------------------------------------
// Purpose: Add / Remove score for this team
//-----------------------------------------------------------------------------
void CTeam::AddScore( int iScore )
{
	m_iScore += iScore;
}

void CTeam::SetScore( int iScore )
{
	m_iScore = iScore;
}

//-----------------------------------------------------------------------------
// Purpose: Get this team's score
//-----------------------------------------------------------------------------
int CTeam::GetScore( void )
{
	return m_iScore;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeam::ResetScores( void )
{
	SetScore(0);
}

int CTeam::GetAliveMembers( void )
{
	int iAlive = 0;

	int iNumPlayers = GetNumPlayers();

	for ( int i=0;i<iNumPlayers;i++ )
	{
		if ( GetPlayer(i) && GetPlayer(i)->IsAlive() )
		{
			iAlive++;
		}
	}

	return iAlive;
}
