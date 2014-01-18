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

#define OBSTACLE_DMG_VALUE 100.0f
#define OBSTACLE_TOUCH_INT 0.2f

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
		void ObsTouch(CBaseEntity *e);

	private:
		CUtlVector<EHANDLE> touching;

		float m_flNextTouch;
#endif

};

#endif

