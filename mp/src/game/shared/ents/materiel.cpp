#include "cbase.h"
#include "materiel.h"
#include "class_info.h"
#include "item_info.h"
#include "ammodef.h"

CMateriel::CMateriel(int team, struct item_info_t *info) : CBaseAnimating() {
	ChangeTeam(team);
	item_info = info;
	active = false;
	dmg_sprite = NULL;
	last_dmg_sprite = 0.0f;
#ifndef CLIENT_DLL
	creator.Term();
#endif
}

CMateriel::~CMateriel(void) {}

#define DMG_SPRITE_INT 0.5f

IMPLEMENT_NETWORKCLASS_ALIASED( Materiel, DT_Materiel )

BEGIN_NETWORK_TABLE( CMateriel, DT_Materiel )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( active ) ),
#else
	SendPropBool( SENDINFO( active ) ),
#endif
END_NETWORK_TABLE()


#ifndef CLIENT_DLL

BEGIN_DATADESC(CMateriel)
END_DATADESC()

void CMateriel::Precache(void) {
	PrecacheModel(item_info->model);
	
	if (item_info->create_sound) {
		PrecacheScriptSound(item_info->create_sound);
	}
	
	if (item_info->killed_sound) {
		PrecacheScriptSound(item_info->killed_sound);
	}

	if (gib1)
		PrecacheModel(gib1);
	if (gib2)
		PrecacheModel(gib2);
	if (gib3)
		PrecacheModel(gib3);
	if (gib4)
		PrecacheModel(gib4);
	if (gib5)
		PrecacheModel(gib5);

	if (dmg_sprite)
		PrecacheParticleSystem(dmg_sprite);

	last_dmg_sprite = 0.0f;

	BaseClass::Precache();
}

void CMateriel::EndTouch(CBaseEntity *e) {
	CBaseEntity *groundEnt;

	groundEnt = GetGroundEntity();
	if (groundEnt && groundEnt->edict() == e->edict()) {
		SetGroundEntity(NULL);
	}

	BaseClass::EndTouch(e);
}

void CMateriel::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	SetModel(item_info->model);
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetCollisionGroup(COLLISION_GROUP_PLAYER);

	SetMaxHealth(item_info->max_health);
	SetHealth(item_info->initial_health);

	m_takedamage = DAMAGE_YES;
	
	(void)VPhysicsInitStatic();

	GetTeam()->spend_points(item_info->value);

	active = false;
}

void CMateriel::DoSpawnSound(void) {
	Vector origin;
	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;

	if (item_info->create_sound != NULL) {
		if (GetParametersForSound( item_info->create_sound, params, NULL )) {
			origin = GetAbsOrigin();

			filter.AddRecipientsByPAS( origin );

			ep.m_nChannel = params.channel;
			ep.m_pSoundName = params.soundname;
			ep.m_flVolume = params.volume;
			ep.m_SoundLevel = params.soundlevel;
			ep.m_nFlags = 0;
			ep.m_nPitch = params.pitch;
			ep.m_pOrigin = &origin;

			EmitSound( filter, entindex(), ep );
		}
	}
}

CHL2MP_Player *CMateriel::GetCreator(void) {
	return dynamic_cast<CHL2MP_Player *>(creator.Get());
}

void CMateriel::SetCreator(CHL2MP_Player *player) {
	creator = player;
}

void CMateriel::EnableEntity(void) {
	active = true;
}

void CMateriel::DisableEntity(void) {
	active = false;
}

void CMateriel::Event_Killed(const CTakeDamageInfo &info) {
	CBaseEntity *attacker;
	CHL2MP_Player *player;
	Vector origin;
	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;

	DisableEntity();

	if (item_info->killed_sound != NULL) {
		if (GetParametersForSound( item_info->killed_sound, params, NULL )) {
			origin = GetAbsOrigin();

			filter.AddRecipientsByPAS( origin );

			ep.m_nChannel = params.channel;
			ep.m_pSoundName = params.soundname;
			ep.m_flVolume = params.volume;
			ep.m_SoundLevel = params.soundlevel;
			ep.m_nFlags = 0;
			ep.m_nPitch = params.pitch;
			ep.m_pOrigin = &origin;

			EmitSound( filter, entindex(), ep );
		}
	}

	BaseClass::Event_Killed(info);
	GetTeam()->reclaim_points(item_info->value);

	attacker = info.GetAttacker();
	if (attacker) {
		player = ToHL2MPPlayer(attacker);
		if (player) {
			if (player->GetTeamNumber() != GetTeamNumber()) {
				if (item_info->team_dmg == 1) {
					// only eggs and teles give points
					player->AddPlayerPoints(1);
				}
			} else {
				if (player->m_iClassNumber != CLASS_ENGINEER_IDX &&
					player->m_iClassNumber != CLASS_BREEDER_IDX) {
						//
						// engy/breeder can destroy materiel without
						// taking any penalty
						//
						player->SubtractPlayerPoints(1);
				}
			}
		}
	}

	if (gib1 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib1, 10.0f);
	if (gib2 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib2, 10.0f);
	if (gib3 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib3, 15.0f);
	if (gib4 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib4, 10.0f);
	if (gib5 != NULL)
		CGib::SpawnSpecificGibs(this, 1, 100, 500, gib5, 15.0f);
}

void CMateriel::RefundPoints(void) {
	GetTeam()->reclaim_points(item_info->value);
}

//
// Notable ammo/dmg types:
// mech = DMG_SHOCK
// exterm = DMG_PLASMA
// 357 = BULLET & ALWAYSGIB
//
int CMateriel::OnTakeDamage(const CTakeDamageInfo &info) {
	int ret;
	int health;
	int attack_weapon;
	float hull_height;
	CHL2MP_Player *p;
	CTakeDamageInfo newinfo = info;

	ret = 0;
	attack_weapon = GetAmmoDef()->AttackWeapon(info.GetAmmoType());
	p = ToHL2MPPlayer(info.GetAttacker());

	//
	// even items that don't take team damage should be
	// damaged by explosions
	//
  	if (p && p->GetTeamNumber() == GetTeamNumber() && item_info->team_dmg == 0) {
		if (!(info.GetDamageType() & DMG_BLAST))
			return 0;
	}

	newinfo.ScaleDamage(item_info->armor_factor);

	if (GetTeamNumber() == TEAM_HUMANS) {
		switch (attack_weapon) {
			case WPN_ENGY:
			case WPN_PISTOL:
			case WPN_SMG:
			case WPN_SHOTGUN_BUCK:
			case WPN_PLASMA_RIFLE:
			case WPN_HATCHY_SLASH:
			case WPN_KAMI_SLASH:
				if (item_info->idx == ITEM_TELEPORTER_IDX)
					return 0;
				break;

			case WPN_RPG:
			case WPN_FRAG:
			case WPN_KAMI_XP:
			case WPN_SHOTGUN_XP:
				newinfo.ScaleDamage(1.3f);
				break;

			case WPN_ACID_GREN:
			case WPN_GASSER:
				newinfo.ScaleDamage(0.2f);
				break;

			case WPN_STINGER_FIRE:
				newinfo.ScaleDamage(0.5f);
				break;

			case WPN_STALKER_SLASH:
			case WPN_GUARDIAN_SLASH:
			case WPN_STINGER_SLASH:
				newinfo.ScaleDamage(0.7f);
				break;
		}
	} else {
		switch (attack_weapon) {
			case WPN_RPG:
			case WPN_FRAG:
			case WPN_KAMI_XP:
			case WPN_SHOTGUN_XP:
			case WPN_PLASMA_CANON:
			case WPN_357:
				newinfo.ScaleDamage(0.8f);
				break;

			case WPN_HATCHY_SLASH:
			case WPN_KAMI_SLASH:
			case WPN_DRONE_SLASH:
			case WPN_STINGER_SLASH:
			case WPN_GUARDIAN_SLASH:
			case WPN_STALKER_SLASH:
			case WPN_STALKER_SPIKE:
			case WPN_GUARDIAN_SPIKE:
			case WPN_STINGER_FIRE:
			case WPN_RAILGUN:
				if (item_info->idx == ITEM_EGG_IDX)
					newinfo.ScaleDamage(0.2f);
				break;

			case WPN_ACID_GREN:
			case WPN_GASSER:
			case WPN_SPIKER:
			case WPN_INFESTED:
				return 0;
				break;
		}
	}

	ret = BaseClass::OnTakeDamage(newinfo);

	health = GetHealth();

	if (health < item_info->min_health) {
		if (active) {
			DisableEntity();
		}
	} else if (health >= item_info->min_health) {
		if (!active) {
			EnableEntity();
		}
	}

	if (ret > 0 && dmg_sprite) {
		if (last_dmg_sprite < gpGlobals->curtime) {
			hull_height = CollisionProp()->OBBMaxs().z;
			DispatchParticleEffect(dmg_sprite, GetAbsOrigin() + Vector(random->RandomFloat(-10.0f, 10.0f), random->RandomFloat(-10.0f, 10.0f), hull_height -2.0f), GetAbsAngles(), this);
			last_dmg_sprite = gpGlobals->curtime + DMG_SPRITE_INT;
		}
	}

	return ret;

}

int CMateriel::TakeHealth(int amt, int type) {
	int health;
	int ret;

	ret = BaseClass::TakeHealth(amt, type);
	
	health = GetHealth();

	if (health < item_info->min_health) {
		if (active) {
			DisableEntity();
		}
	} else if (health >= item_info->min_health) {
		if (!active) {
			EnableEntity();
		}
	}

	return ret;
}

#else

int CMateriel::DrawModel(int flags) {
	C_HL2MP_Player *local;

	shouldGlow = false;

	local = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (local && local->GetTeamNumber() == TEAM_SPIDERS) {
		shouldGlow = true;
		if (GetTeamNumber() == TEAM_HUMANS && local->m_iClassNumber == CLASS_COMMANDO_IDX
			&& GetAbsVelocity() == vec3_origin) {
				shouldGlow = false;
		}
	}

	return BaseClass::DrawModel(flags);
}

void CMateriel::OnDataChanged(DataUpdateType_t type) {
	BaseClass::OnDataChanged(type);
	if (type == DATA_UPDATE_CREATED) {
		active_cache = active;
		if (active) {
			EnableEntity();
		} else {
			DisableEntity();
		}
		return;
	}

	if (active != active_cache) {
		active_cache = active;
		if (active) {
			EnableEntity();
		} else {
			DisableEntity();
		}
	}
}

void CMateriel::EnableEntity(void) {
}

void CMateriel::DisableEntity(void) {
}

#endif