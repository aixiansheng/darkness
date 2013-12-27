//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"
#include "hl2mp_gameinterface.h"
#include "team.h"
#include "team_spawnpoint.h"
#include "ents/teleporter.h"
#include "ents/egg.h"
#include "ents/point_set.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameClients implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameClients::GetPlayerLimits( int& minplayers, int& maxplayers, int &defaultMaxPlayers ) const
{
	minplayers = defaultMaxPlayers = 2; 
	maxplayers = 16;
}

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameDLL implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameDLL::LevelInit_ParseAllEntities( const char *pMapEntities )
{
	// find spider team and human team in g_Teams...
	int i;
	CTeam *t;
	CBaseEntity *e;
	CTeleporterEntity *tele;
	CEggEntity *egg;

	g_SpiderTeam = NULL;
	g_HumanTeam = NULL;
	g_PointSet = NULL;

	e = NULL;
	g_PointSet = (CPointSet *)gEntList.FindEntityByClassname(e, "point_set");
	
	if (g_PointSet == NULL) {
		g_PointSet = (CPointSet *)CreateEntityByName("point_set");
		if (g_PointSet == NULL) {
			Error("Unable to find/generate CPointSet\n");
		}
	}

	for (i = 0; i < GetNumberOfTeams(); i++) {
		if ((t = GetGlobalTeam(i)) == NULL)
			break;
		if (stricmp(t->GetName(), SPIDER_TEAM_NAME) == 0) {
			g_SpiderTeam = t;
			g_SpiderTeam->InitPoints();
		} else if (stricmp(t->GetName(), HUMAN_TEAM_NAME) == 0) {
			g_HumanTeam = t;
			g_HumanTeam->InitPoints();
		}
	}

	if (!g_SpiderTeam || !g_HumanTeam) {
		Error("Unable to find Spider/Human teams!\n");
	}

	i = 0;
	e = NULL;
	while ((e = gEntList.FindEntityByClassname(e, "ent_egg")) != NULL) {
		egg = dynamic_cast<CEggEntity *>(e);
		if (egg) {
			/*s->ChangeTeam(g_SpiderTeam->GetTeamNumber());*/
			i++;
		}
	}

	if (i == 0) {
		Error("Unable to find Spider spawns\n");
	}

	i = 0;
	e = NULL;
	while ((e = gEntList.FindEntityByClassname(e, "ent_teleporter")) != NULL) {
		tele = dynamic_cast<CTeleporterEntity *>(e);
		if (tele) {
			/*s->ChangeTeam(g_HumanTeam->GetTeamNumber());*/
			i++;
		}
	}

	if (i == 0) {
		Error("Unable to find Human spawns\n");
	}
}

