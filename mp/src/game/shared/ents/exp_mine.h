#ifndef EXP_MINE_H
#define EXP_MINE_H

#include "cbase.h"
#include "human_materiel.h"

#ifdef CLIENT_DLL
	#define CExpMineEntity C_ExpMineEntity
#else
	#include "team_spawnpoint.h"
	#include "item_info.h"
	#include "team_spawnpoint.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
	#include "item_info.h"
	#include "explode.h"
	#include "beam_shared.h"
#endif

#define EXPMINE_RADIUS 300
#define EXPMINE_DAMAGE 300
#define EXPMINE_THINK_INTERVAL 0.05f
#define EXPMINE_ARM_TIME 3.0f

class CExpMineEntity : public CHumanMateriel {
	public:
		DECLARE_CLASS(CExpMineEntity, CHumanMateriel);
		DECLARE_NETWORKCLASS();

		CExpMineEntity(void);
		~CExpMineEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		void Spawn(void);
		void SetNormal(Vector &normal);
		void SetupThink(void);
		void DetectThink(void);
		void SolidThink(void);
		virtual void Event_Killed(const CTakeDamageInfo &info);

		Vector endpos;
		Vector surfaceNorm;
		CBeam *laser;

#endif

};

#endif
