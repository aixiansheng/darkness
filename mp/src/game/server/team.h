//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef TEAM_H
#define TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "utlvector.h"
#include "ents/point_set.h"

#define SPIDER_TEAM_NAME "Spiders"
#define HUMAN_TEAM_NAME "Humans"

#define STUB_POINT_VALUE 9000

extern CTeam *g_SpiderTeam;
extern CTeam *g_HumanTeam;
extern CPointSet *g_PointSet;

class CBasePlayer;
class CTeamSpawnPoint;

class CTeam : public CBaseEntity
{
	DECLARE_CLASS( CTeam, CBaseEntity );
	DECLARE_SERVERCLASS();

public:

	CTeam( void );
	virtual ~CTeam( void );

	virtual void Precache( void ) { return; };
	virtual void Think( void );
	virtual int  UpdateTransmitState( void );

	//-----------------------------------------------------------------------------
	// Initialization
	//-----------------------------------------------------------------------------
	virtual void Init( const char *pName, int iNumber );
	void InitPoints(void);

	//-----------------------------------------------------------------------------
	// Data Handling
	//-----------------------------------------------------------------------------
	virtual int			GetTeamNumber( void ) const;
	virtual const char *GetName( void );
	virtual void		UpdateClientData( CBasePlayer *pPlayer );
	virtual bool		ShouldTransmitToPlayer( CBasePlayer* pRecipient, CBaseEntity* pEntity );

	//-----------------------------------------------------------------------------
	// Spawnpoints
	//-----------------------------------------------------------------------------
	virtual void InitializeSpawnpoints( void );
	virtual void AddSpawnpoint( CTeamSpawnPoint *pSpawnpoint );
	virtual void RemoveSpawnpoint( CTeamSpawnPoint *pSpawnpoint );
	virtual CBaseEntity *SpawnPlayer( CBasePlayer *pPlayer );
	virtual bool HasSpawnPoints(void);

	//-----------------------------------------------------------------------------
	// Players
	//-----------------------------------------------------------------------------
	virtual void InitializePlayers( void );
	virtual void AddPlayer( CBasePlayer *pPlayer );
	virtual void RemovePlayer( CBasePlayer *pPlayer );
	virtual int  GetNumPlayers( void );
	virtual CBasePlayer *GetPlayer( int iIndex );

	//-----------------------------------------------------------------------------
	// Scoring
	//-----------------------------------------------------------------------------
	virtual void AddScore( int iScore );
	virtual void SetScore( int iScore );
	virtual int  GetScore( void );
	virtual void ResetScores( void );

	// Round scoring
	virtual int GetRoundsWon( void ) { return m_iRoundsWon; }
	virtual void SetRoundsWon( int iRounds ) { m_iRoundsWon = iRounds; }
	virtual void IncrementRoundsWon( void ) { m_iRoundsWon++; }

	void EnqueueSpawn(CBasePlayer *p);
	void RespawnPlayer(void);

	virtual int GetAliveMembers( void );

public:
	CUtlVector< CTeamSpawnPoint * > m_aSpawnPoints;
	CUtlVector< CBasePlayer * > m_aPlayers;
	CUtlVector< EHANDLE > spawnQueue;

	// Data
	CNetworkString( m_szTeamname, MAX_TEAM_NAME_LENGTH );
	CNetworkVar( int, m_iScore );
	CNetworkVar( int, m_iRoundsWon );
	CNetworkVar( int, num_spawns );

	int		m_iDeaths;

	// Spawnpoints
	int m_iLastSpawn;		// Index of the last spawnpoint used

	bool spend_points(int points);
	void reclaim_points(int points);

	CNetworkVar( int, asset_points );
	CNetworkVar( int, m_iTeamNum );			// Which team is this?

	float m_flNextRespawnCheck;
};

extern CUtlVector< CTeam * > g_Teams;
extern CTeam *GetGlobalTeam( int iIndex );
extern int GetNumberOfTeams( void );

#endif // TEAM_H
