#ifndef __HUMAN_MATERIEL_H__
#define __HUMAN_MATERIEL_H__

#include "cbase.h"
#include "materiel.h"

#ifdef CLIENT_DLL
	#define CHumanMateriel C_HumanMateriel
#else
	#include "particle_parse.h"
#endif

class CHumanMateriel : public CMateriel {

public:
	DECLARE_CLASS (CHumanMateriel, CMateriel)
	
	CHumanMateriel(struct item_info_t *info);
	~CHumanMateriel();

#ifndef CLIENT_DLL

	DECLARE_DATADESC();

	virtual void Spawn(void);
	virtual void Precache(void);

	void SelfDestructThink(void);
	void DamagedThink(void);

#endif

};

#endif
