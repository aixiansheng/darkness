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

#define HEALER_HEAL_VALUE		1.0f
#define HEALER_THINK_INTERVAL	0.5f

#define HEALER_GARGLE_SOUND "Healer.Gargle"
#define HEALER_GARGLE_INTERVAL 3.0f
#define HEALER_RADIUS 64.0f

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
		virtual void HealThink(void);

		float next_gargle;
		unsigned int healed_total;

#endif

		
		
};

#endif

