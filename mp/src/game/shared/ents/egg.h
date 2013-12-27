#ifndef EGG_H__
#define EGG_H__

#include "cbase.h"
#include "spider_materiel.h"

#ifdef CLIENT_DLL
	#define CEggEntity C_EggEntity
#else
	#include "team_spawnpoint.h"
	#include "team.h"
	#include "hl2mp_gamerules.h"
#endif

#define EGG_SPAWN_PLAYER_SOUND "Egg.PlayerSpawn"

class CEggEntity : public CSpiderMateriel {
	public:
		DECLARE_CLASS(CEggEntity, CSpiderMateriel);
		DECLARE_NETWORKCLASS();
		
		CEggEntity(void);
		~CEggEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		virtual void Spawn(void);
		virtual void Precache(void);

		void InputToggle(inputdata_t &input);
		void Event_Killed(const CTakeDamageInfo &info);

		void EntNoClip(CBaseEntity *e);
		bool ShouldCollideWithEntity(const CBaseEntity *e) const;

		void NoClipThink(void);

		void SpawnSound(void);

		CTeamSpawnPoint *SpawnPoint(void);

	private:
		CTeamSpawnPoint *spawnpoint;
		CUtlVector<EHANDLE> noclip_ents;
#endif

};

#endif