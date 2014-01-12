#ifndef EXP_SPIKER_H
#define EXP_SPIKER_H

#include "cbase.h"
#include "spider_materiel.h"

#ifdef CLIENT_DLL
	#define CSpikerEntity C_SpikerEntity
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
	#include "spike.h"
#endif

#define SPIKER_RADIUS 580
#define SPIKER_DAMAGE 110
#define SPIKER_THINK_INTERVAL 0.1f
#define SPIKER_ARM_TIME 2.0f
#define SPIKER_RECHARGE_TIME 4.0f

#define SPIKER_AMMO_TYPE "spike"
#define SPIKER_SHOT_SOUND "Weapon_Crossbow.Single"

class CSpikerEntity : public CSpiderMateriel {
	public:
		DECLARE_CLASS(CSpikerEntity, CSpiderMateriel);
		DECLARE_NETWORKCLASS();

		CSpikerEntity(void);
		~CSpikerEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);
		void SetInitialHealth(void);

		virtual void Event_Killed(const CTakeDamageInfo &info);

		void SetupThink(void);
		void DetectThink(void);
		void RechargeThink(void);

		void ShootSpike(Vector origin, Vector v);
		void ShootProjectileSpike(Vector origin, Vector v);
		void Explode(CBaseEntity *ent, int bitsDamageType);

		void Shoot4Spikes(void);

		void SetSpikes(void);

		int ammotype;
		
		CSpike *s1;
		CSpike *s2;
		CSpike *s3;
		CSpike *s4;

#endif

};

#endif
