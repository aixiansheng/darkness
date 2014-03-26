#ifndef __MATERIEL_H__
#define __MATERIEL_H__

#include "cbase.h"
#include "item_info.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_baseanimating.h"
	#define CMateriel C_Materiel
#else
	#include "particle_parse.h"
	#include "team.h"
	#include "gib.h"
	#include "hl2mp_player.h"
#endif

class CMateriel : public CBaseAnimating {

public:

	DECLARE_CLASS (CMateriel, CBaseAnimating);
	DECLARE_NETWORKCLASS();

	CMateriel(int team = TEAM_HUMANS, struct item_info_t *info = &human_items[0]);
	~CMateriel(void);

	struct item_info_t *item_info;

	virtual void EnableEntity(void);
	virtual void DisableEntity(void);

#ifndef CLIENT_DLL
	
	DECLARE_DATADESC();

	virtual void Event_Killed(const CTakeDamageInfo &info);
	virtual int OnTakeDamage(const CTakeDamageInfo &info);

	virtual int TakeHealth(int amt, int type);

	virtual void Spawn(void);
	virtual void Precache(void);
	virtual CHL2MP_Player *GetCreator(void);
	virtual void SetCreator(CHL2MP_Player *player);
	virtual void EndTouch(CBaseEntity *e);

	CNetworkVar(bool, active);

	CHandle<CHL2MP_Player> creator;

#else

	virtual int DrawModel(int flags);
	virtual void OnDataChanged(DataUpdateType_t type);

	bool active;
	bool active_cache;

#endif

	const char *gib1;
	const char *gib2;
	const char *gib3;
	const char *gib4;
	const char *gib5;

	const char *dmg_sprite;

	float last_dmg_sprite;

};

#endif
