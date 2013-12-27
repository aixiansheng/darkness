#include "cbase.h"
#include "ammo_crate.h"
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( ent_ammo_crate, CAmmoCrate );
IMPLEMENT_NETWORKCLASS_ALIASED( AmmoCrate, DT_AmmoCrate );

BEGIN_NETWORK_TABLE( CAmmoCrate, DT_AmmoCrate )
END_NETWORK_TABLE()

CAmmoCrate::CAmmoCrate() : CHumanMateriel(&human_items[ITEM_AMMO_CRATE_IDX]) {}
CAmmoCrate::~CAmmoCrate() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC( CAmmoCrate )
	DEFINE_THINKFUNC( CrateThink ),
END_DATADESC()

void CAmmoCrate::Spawn( void ) {
	Precache();
	BaseClass::Spawn();

	SetThink(&CAmmoCrate::CrateThink);
	SetNextThink(gpGlobals->curtime + AMMO_CRATE_THINK_TIME);

	refils_total = 0;
}

int CAmmoCrate::ObjectCaps(void) {
	return BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
}

void CAmmoCrate::OnRestore( void ) {
	BaseClass::OnRestore();
}

void CAmmoCrate::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) {
	CHL2MP_Player *pPlayer;
	float wait_time;
	char buf[64];

	pPlayer = ToHL2MPPlayer( pActivator );
	if (active == false || pPlayer == NULL || !pPlayer->IsAlive())
		return;

	CDisablePredictionFiltering foo;

	if (PlayerIsWaiting(pPlayer, &wait_time)) {
		Q_snprintf(buf, sizeof(buf), "Please wait %.0f more seconds\n", wait_time - gpGlobals->curtime);
		UTIL_SayText(buf, pPlayer);
		return;
	}

	MakePlayerWait(pPlayer, AMMO_CRATE_RELOAD_TIME);
	
	pPlayer->RefilAmmo(false);
	refils_total++;

	// reward creator every 10 refils
	if (refils_total % 10 == 0 && GetCreator() && GetCreator()->GetTeamNumber() == GetTeamNumber()) {
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

	UTIL_HudHintText(p, "Press USE [ %+use% ] to refil ammo");
}

void CAmmoCrate::CrateThink( void ) {
	int i;
	EHANDLE ent;
	CHL2MP_Player *p;

	for (i = 0; i < waiting.Count(); i++) {
		ent = waiting[i];
		p = dynamic_cast<CHL2MP_Player *>(ent.Get());
		if (!ent || !p) {
			waiting.Remove(i);
		}

		if (p->next_ammo_pickup < gpGlobals->curtime) {
			waiting.Remove(i);
		}
	}

	SetNextThink(AMMO_CRATE_THINK_TIME);
}

bool CAmmoCrate::PlayerIsWaiting(CHL2MP_Player *p, float *wait_time) {
	EHANDLE ent;
	
	ent = p;
	if (waiting.Find(ent) == waiting.InvalidIndex()) {
		return false;
	}

	*wait_time = p->next_ammo_pickup;

	return true;
}

void CAmmoCrate::MakePlayerWait(CHL2MP_Player *p, float wait_time) {
	EHANDLE ent;
	
	ent = p;

	if (waiting.Find(ent) == waiting.InvalidIndex()) {
		waiting.AddToTail(ent);
		p->next_ammo_pickup = gpGlobals->curtime + wait_time;
	}
}

#endif