#ifndef __SPIDER_MATERIEL_H__
#define __SPIDER_MATERIEL_H__

#include "cbase.h"
#include "materiel.h"

#define SELF_HEAL_CTX "SelfHeal"
#define SELF_HEAL_INTERVAL 3.0f
#define DEFAULT_HEAL_AMT 1

#ifdef CLIENT_DLL
	#define CSpiderMateriel C_SpiderMateriel
#endif

class CSpiderMateriel : public CMateriel {

public:
	DECLARE_CLASS (CSpiderMateriel, CMateriel)

	CSpiderMateriel(struct item_info_t *info);
	~CSpiderMateriel();

#ifndef CLIENT_DLL

	DECLARE_DATADESC();

	virtual void Spawn(void);
	virtual void Precache(void);

	void SetSelfHealAmt(int amt);
	void SelfHealThink(void);

	virtual void Event_Killed(const CTakeDamageInfo &info);

	virtual void StartTouch(CBaseEntity *e);

private:
	float healAmount;
	float nextNudge;

#endif

};

#endif
