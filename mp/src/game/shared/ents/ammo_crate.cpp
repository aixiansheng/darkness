#include "cbase.h"
#include "ammo_crate.h"
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( ent_ammo_crate, CAmmoCrate );
IMPLEMENT_NETWORKCLASS_ALIASED( AmmoCrate, DT_AmmoCrate );

BEGIN_NETWORK_TABLE( CAmmoCrate, DT_AmmoCrate )
END_NETWORK_TABLE()

CAmmoCrate::CAmmoCrate() : CHumanMateriel(&dk_items[ITEM_AMMO_CRATE_IDX]) {}
CAmmoCrate::~CAmmoCrate() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC( CAmmoCrate )
END_DATADESC()

void CAmmoCrate::Spawn( void ) {
	Precache();
	BaseClass::Spawn();

	refils_total = 0;
}

int CAmmoCrate::ObjectCaps(void) {
	return BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
}

void CAmmoCrate::OnRestore( void ) {
	BaseClass::OnRestore();
}

void CAmmoCrate::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) {
	CDisablePredictionFiltering pfilter;
	CHL2MP_Player *pPlayer;

	pPlayer = ToHL2MPPlayer( pActivator );
	if (!pPlayer)
		return;

	if (active == false || !pPlayer->IsAlive())
		return;

	if (pPlayer->EntRefil(AMMO_CRATE_RELOAD_TIME) == true) {
		refils_total++;
	}

	// reward creator every 10 refils
	if (refils_total % 10 == 0 &&
		GetCreator() &&
		GetCreator()->GetTeamNumber() == GetTeamNumber()) 
	{
		GetCreator()->IncrementFragCount(1);
	}
}

void CAmmoCrate::StartTouch(CBaseEntity *ent) {
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(ent);
	if (p == NULL)
		return;

	if (p->GetTeamNumber() != TEAM_HUMANS)
		return;

	UTIL_HudHintText(p, "#DK_Hint_AmmoCrate");
}

#endif