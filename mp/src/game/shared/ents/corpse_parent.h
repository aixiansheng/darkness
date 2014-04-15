#ifndef __CORPSE_PARENT_H__
#define __CORPSE_PARENT_H__

#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_baseanimating.h"
	#define CCorpseParent C_CorpseParent
#else
	#include "team.h"
	#include "hl2mp_player.h"
#endif


#define CORPSE_MODEL		"models/infested.mdl"

class CCorpseParent : public CBaseAnimating {

public:

	DECLARE_CLASS(CCorpseParent, CBaseAnimating);
	DECLARE_NETWORKCLASS();

	CCorpseParent();
	~CCorpseParent(void);

#ifndef CLIENT_DLL
	
	DECLARE_DATADESC();

	virtual void Spawn(void);

#endif

};

#endif
