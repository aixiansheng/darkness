#ifndef __INFESTED_CORPSE_H__
#define __INFESTED_CORPSE_H__

#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_baseanimating.h"
	#define CInfestedCorpse C_InfestedCorpse
#else
	#include "team.h"
	#include "gib.h"
	#include "particle_parse.h"
	#include "hl2mp_player.h"
#endif


#define CORPSE_GIB_SOUND	"Egg.Die"
#define CORPSE_MODEL		"models/infested.mdl"
#define CORPSE_FLIES_SPRITE	"black_flies"
#define CORPSE_HEALTH		275
#define CORPSE_FLIES_SOUND	"Corpse.Flies"
#define CORPSE_DMG_INT		0.3f
#define CORPSE_DMG_RAD		200
#define CORPSE_DMG_VAL		20

#define CORPSE_SND_INT		0.7f
#define CORPSE_SND_CTX		"corpse_snd"

class CInfestedCorpse : public CBaseAnimating {

public:

	DECLARE_CLASS (CInfestedCorpse, CBaseAnimating);
	DECLARE_NETWORKCLASS();

	CInfestedCorpse();
	~CInfestedCorpse(void);

#ifndef CLIENT_DLL
	
	DECLARE_DATADESC();

	virtual void Event_Killed(const CTakeDamageInfo &info);
	virtual void Spawn(void);
	virtual void Precache(void);
	void SoundThink(void);
	void DamageThink(void);
	virtual CHL2MP_Player *GetCreator(void);
	virtual CHL2MP_Player *GetCreator(void) const;
	virtual void SetCreator(CHL2MP_Player *player);

	CHandle<CHL2MP_Player> creator;

#else

	virtual void UpdateOnRemove(void);
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual int DrawModel(int flags);
	void ShowFlies(void);

	CNewParticleEffect *flies;

#endif

	const char *gib1;
	const char *gib2;
	const char *gib3;
	const char *gib4;
	const char *gib5;

};

#endif
