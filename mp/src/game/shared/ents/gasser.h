#ifndef _GASSER_H
#define _GASSER_H

#include "cbase.h"
#include "spider_materiel.h"

#ifdef CLIENT_DLL
	#define CGasserEntity C_GasserEntity
#else
	#include "team_spawnpoint.h"
	#include "item_info.h"
	#include "team_spawnpoint.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
	#include "item_info.h"
	#include "explode.h"
	#include "soundent.h"
	#include "ammodef.h"
#endif


#define GASSER_RECHARGE_TIME 10.0f

class CGasserEntity : public CSpiderMateriel {
	public:
		DECLARE_CLASS(CGasserEntity, CSpiderMateriel);
		DECLARE_NETWORKCLASS();

		CGasserEntity(void);
		~CGasserEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		void Spawn(void);

		void RechargeThink(void);

		void MakeGas(void);
#endif

};

#endif
