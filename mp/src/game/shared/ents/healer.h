#ifndef HEALER_H_
#define HEALER_H_

#include "cbase.h"
#include "spider_materiel.h"

#ifdef CLIENT_DLL
	#define CHealerEntity C_HealerEntity
#else
	#include "team_spawnpoint.h"
	#include "item_info.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
#endif

#define HEALER_HEAL_VALUE 1.0f
#define HEALER_HEAL_INTERVAL 0.3f
#define HEALER_INACTIVE_THINK_INTERVAL 1.0f
#define HEALER_ACTIVE_THINK_INVERVAL 0.3f

#define HEALER_GARGLE_SOUND "Healer.Gargle"
#define HEALER_GARGLE_INTERVAL 3.0f

#define HEAL_THROTTLE_CTX "heal_throttle"
#define HEAL_THROTTLE_INT 1.5f
#define HEAL_RADIUS 64.0f

class CHealerEntity : public CSpiderMateriel {
	public:
		DECLARE_CLASS(CHealerEntity, CSpiderMateriel);
		DECLARE_NETWORKCLASS();
		
		CHealerEntity(void);
		~CHealerEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		virtual void Spawn(void);
		virtual void Precache(void);

		void HealThink(void);
		void HealThrottleThink(void);

		float next_gargle;
		unsigned int healed_total;

		CUtlVector<EHANDLE> touching;
#endif

		
		
};

#endif

