#ifndef MEDIPAD_H_
#define MEDIPAD_H_

#include "cbase.h"
#include "human_materiel.h"

#ifdef CLIENT_DLL
	#define CMedipadEntity C_MedipadEntity
#else
	#include "team_spawnpoint.h"
	#include "item_info.h"
	#include "team_spawnpoint.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
#endif

#define MEDIPAD_HEAL_VALUE 1.0f
#define MEDIPAD_THINK_INTERVAL 1.0f
#define MIN_MEDIPAD_HEALTH 120

class CMedipadEntity : public CHumanMateriel {
	public:
		DECLARE_CLASS(CMedipadEntity, CHumanMateriel);
		DECLARE_NETWORKCLASS();

		CMedipadEntity(void);
		~CMedipadEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		void Spawn(void);

		void InputToggle(inputdata_t &input);

		void HealThink(void);
		virtual void StartTouch(CBaseEntity *e);
		virtual void EndTouch(CBaseEntity *e);
		
	private:
		
		unsigned int healed_total;
		CUtlVector<EHANDLE> touching;

#endif

};

#endif

