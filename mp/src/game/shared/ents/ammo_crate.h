#ifndef HL2MP_AMMO_CRATE_H
#define HL2MP_AMMO_CRATE_H
#pragma once

#include "cbase.h"
#include "human_materiel.h"

#ifdef CLIENT_DLL
	#define CAmmoCrate C_AmmoCrate
#else
	#include "player.h"
	#include "hl2mp_player.h"
	#include "gamerules.h"
	#include "items.h"
	#include "ammodef.h"
	#include "eventlist.h"
	#include "npcevent.h"
	#include "item_info.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
#endif

#define AMMO_CRATE_RELOAD_TIME	90.0f
#define AMMO_CRATE_THINK_TIME	5.0f
#define AMMO_CRATE_DENY_SOUND	"AmmoCrate.Deny"

// Ammo crate
class CAmmoCrate : public CHumanMateriel {
public:
	DECLARE_CLASS( CAmmoCrate, CHumanMateriel );
	DECLARE_NETWORKCLASS();

	CAmmoCrate(void);
	~CAmmoCrate(void);

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	void	Spawn( void );
	void	SetupCrate( void );
	void	OnRestore( void );
	void	Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int		ObjectCaps(void);
	virtual void StartTouch(CBaseEntity *ent);

protected:

	unsigned int refils_total;

#endif

};

#endif