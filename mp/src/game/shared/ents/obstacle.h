#ifndef OBSTACLE_H_
#define OBSTACLE_H_

#include "cbase.h"
#include "spider_materiel.h"

#ifdef CLIENT_DLL
	#define CObstacleEntity C_ObstacleEntity
#else
	#include "team_spawnpoint.h"
	#include "item_info.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
#endif


#define OBSTACLE_DMG_VALUE 400.0f
#define OBSTACLE_DMG_INTERVAL 0.1f
#define OBSTACLE_INACTIVE_THINK_INTERVAL 0.3f
#define OBSTACLE_ACTIVE_THINK_INVERVAL 0.1f

class CObstacleEntity : public CSpiderMateriel {
	public:
		DECLARE_CLASS(CObstacleEntity, CSpiderMateriel);
		DECLARE_NETWORKCLASS();

		CObstacleEntity(void);
		~CObstacleEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);

		void DmgThink(void);
		void StartTouch(CBaseEntity *e);
		void EndTouch(CBaseEntity *e);

	private:
		CUtlVector<EHANDLE> touching;
#endif

};

#endif

