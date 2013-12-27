#include "cbase.h"
#include "human_materiel.h"
#include "hl2mp_gamerules.h"

#define SELF_DESTRUCT_TIMEOUT	7.0f
#define DAMAGED_THINK_INT		5.0f
#define SELF_DESTRUCT_CTX		"selfDestruct"
#define DAMAGED_EFFECTS_CTX		"damageEffects"

#define DAMAGED_EFFECT_SOUND	"DamagedMateriel.Sparks"
#define DAMAGE_SPARKS_FX		"sparking_gear"

CHumanMateriel::CHumanMateriel(struct item_info_t *info) : CMateriel(TEAM_HUMANS, info) {
	active = false;
	gib1 = "models/Gibs/Glass_shard01.mdl";
	gib2 = "models/Gibs/Glass_shard02.mdl";
	gib3 = "models/Gibs/Glass_shard03.mdl";
	gib4 = "models/Gibs/manhack_gib01.mdl";
	gib5 = "models/Gibs/manhack_gib02.mdl";
	dmg_sprite = DAMAGE_SPARKS_FX;
}

CHumanMateriel::~CHumanMateriel() {}

#ifndef CLIENT_DLL

BEGIN_DATADESC(CHumanMateriel)
		DEFINE_THINKFUNC(SelfDestructThink),
		DEFINE_THINKFUNC(DamagedThink),
END_DATADESC()

void CHumanMateriel::Spawn(void) {
	BaseClass::Spawn();
	if (item_info->max_health == item_info->initial_health) {
		EnableEntity();
	} else {
		DisableEntity();
	}
	
	RegisterThinkContext(SELF_DESTRUCT_CTX);
	RegisterThinkContext(DAMAGED_EFFECTS_CTX);

	SetContextThink(&CHumanMateriel::DamagedThink, gpGlobals->curtime + DAMAGED_THINK_INT, DAMAGED_EFFECTS_CTX);
	SetContextThink(&CHumanMateriel::SelfDestructThink, gpGlobals->curtime + SELF_DESTRUCT_TIMEOUT, SELF_DESTRUCT_CTX);
}

void CHumanMateriel::Precache(void) {
	PrecacheScriptSound(DAMAGED_EFFECT_SOUND);
	PrecacheParticleSystem(DAMAGE_SPARKS_FX);
	BaseClass::Precache();
}

void CHumanMateriel::SelfDestructThink(void) {
	if (GetHealth() <= item_info->initial_health && item_info->min_health > 0) {
		CTakeDamageInfo info(NULL, NULL, 9999, DMG_BLAST);
		info.SetDamagePosition(GetAbsOrigin());
		info.SetDamageForce(Vector(0,0,10));
		TakeDamage(info);
		ApplyMultiDamage();
	}
}

//
// makes sparks/noises when human stuff
// is damaged/awaiting repair
//
void CHumanMateriel::DamagedThink(void) {
	float hull_height;

	SetNextThink(gpGlobals->curtime + DAMAGED_THINK_INT + random->RandomFloat(-1.0f, 5.0f), DAMAGED_EFFECTS_CTX);

	if (active == false) {
		EmitSound(DAMAGED_EFFECT_SOUND);
		hull_height = CollisionProp()->OBBMaxs().z;
		DispatchParticleEffect(DAMAGE_SPARKS_FX, GetAbsOrigin() + Vector(random->RandomFloat(-5.0f, 5.0f), random->RandomFloat(-5.0f, 5.0f), hull_height + 3.0f), GetAbsAngles(), this);
	}
}

#endif