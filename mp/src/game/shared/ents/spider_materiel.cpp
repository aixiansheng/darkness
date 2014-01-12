#include "cbase.h"
#include "spider_materiel.h"
#include "hl2mp_gamerules.h"
#include "class_info.h"
#include "in_buttons.h"

#define DMG_SPRITE "blood_spurt"

CSpiderMateriel::CSpiderMateriel(struct item_info_t *info) : CMateriel(TEAM_SPIDERS, info) {
	active = true;
	gib1 = "models/Gibs/Antlion_gib_small_2.mdl";
	gib2 = "models/Gibs/AGIBS.mdl";
	gib3 = "models/Gibs/Antlion_gib_small_1.mdl";
	gib4 = "models/Gibs/Antlion_gib_small_3.mdl";
	gib5 = "models/Gibs/Antlion_gib_medium_2.mdl";
	dmg_sprite = "blood_spurt";
}

CSpiderMateriel::~CSpiderMateriel() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CSpiderMateriel)
	DEFINE_THINKFUNC(SelfHealThink),
END_DATADESC()

void CSpiderMateriel::Spawn(void) {
	BaseClass::Spawn();

	healAmount = DEFAULT_HEAL_AMT;

	RegisterThinkContext(SELF_HEAL_CTX);
	SetContextThink(&CSpiderMateriel::SelfHealThink, gpGlobals->curtime + SELF_HEAL_INTERVAL, SELF_HEAL_CTX);

	nextNudge = 0.0f;
}

void CSpiderMateriel::Precache(void) {
	BaseClass::Precache();
	PrecacheParticleSystem(DMG_SPRITE);
}

void CSpiderMateriel::SelfHealThink(void) {
	SetNextThink(gpGlobals->curtime + SELF_HEAL_INTERVAL, SELF_HEAL_CTX);
	TakeHealth(healAmount, DMG_GENERIC); 
}

void CSpiderMateriel::SetSelfHealAmt(int amt) {
	healAmount = amt;
}

void CSpiderMateriel::Event_Killed(const CTakeDamageInfo &info) {
	Vector dir;
	Vector origin;
	Vector traceDir;
	trace_t tr;
	int i;
	CDisablePredictionFiltering foo2;

	origin = WorldSpaceCenter();
	dir = info.GetDamageForce();

	UTIL_BloodSpray(origin, dir, BLOOD_COLOR_RED, 4, FX_BLOODSPRAY_ALL);

	for (i = 0; i < 5; i++) {
		traceDir = dir;

		traceDir.x += random->RandomFloat(-0.1f, 0.1f);
		traceDir.y += random->RandomFloat(-0.1f, 0.1f);
		traceDir.z += random->RandomFloat(-0.1f, 0.1f);

		UTIL_TraceLine(origin, origin + (dir * 175.0f), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction != 1.0) {
			UTIL_BloodDecalTrace(&tr, BLOOD_COLOR_RED);
		}
	}

	BaseClass::Event_Killed(info);
}

#define MIN_BREEDER_NUDGE_SPEED 250.0f
#define MIN_NUDGE_WAIT 1.0f

void CSpiderMateriel::StartTouch(CBaseEntity *e) {
	CHL2MP_Player *p;
	Vector vel;
	Vector itemVel;
	float speed;
	
	p = ToHL2MPPlayer(e);
	if (!p || p->GetTeamNumber() == TEAM_HUMANS || p->m_iClassNumber != CLASS_BREEDER_IDX)
		return;

	vel = e->GetAbsVelocity();
	speed = vel.Length();

	if (speed < MIN_BREEDER_NUDGE_SPEED)
		return;

	if (item_info->idx != ITEM_OBSTACLE_IDX &&
		item_info->idx != ITEM_SPIKER_IDX)
		return;

	if (p->m_nButtons & IN_USE) {

		if (nextNudge < gpGlobals->curtime) {
			itemVel = vel * 1.5f;
			itemVel.z = 15.0f;
			SetAbsVelocity(itemVel);

			nextNudge = gpGlobals->curtime + MIN_NUDGE_WAIT;
		}

	}

	BaseClass::StartTouch(e);
}

#endif
