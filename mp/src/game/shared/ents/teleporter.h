#ifndef TELEPORTER_H_
#define TELEPORTER_H_

#include "cbase.h"
#include "human_materiel.h"

#define TELEPORTER_SPAWN_PLAYER_SOUND "Teleporter.PlayerSpawn"
#define TELEPORTER_SPRITE "teleporter_sparks"

#ifdef CLIENT_DLL
	#include "nv_screeneffect.h"
	#include "c_hl2mp_player.h"
	#include "c_baseplayer.h"
	#include "model_types.h"

	#define CTeleporterEntity C_TeleporterEntity
#else
	#include "team_spawnpoint.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
	#include "item_info.h"
	#include "particle_parse.h"
#endif

class CTeleporterEntity : public CHumanMateriel {
	public:
		DECLARE_CLASS(CTeleporterEntity, CHumanMateriel);
		DECLARE_NETWORKCLASS(); 
				
		CTeleporterEntity();
		~CTeleporterEntity();

		virtual void EnableEntity(void);
		virtual void DisableEntity(void);

#ifndef CLIENT_DLL

		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);

		void InputToggle(inputdata_t &input);
		virtual int OnTakeDamage(const CTakeDamageInfo &info);
		virtual int TakeHealth(int amt, int type);
		virtual void Event_Killed(const CTakeDamageInfo &info);

		virtual void StartTouch(CBaseEntity *e);
		void SpawnSound(void);

		CTeamSpawnPoint *SpawnPoint(void);

	private:
		CTeamSpawnPoint *spawnpoint;
#else
		CNewParticleEffect *sparks;
#endif

};

#endif