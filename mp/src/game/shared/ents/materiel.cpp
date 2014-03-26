#include "cbase.h"
#include "materiel.h"
#include "class_info.h"
#include "item_info.h"

CMateriel::CMateriel(int team = TEAM_HUMANS, struct item_info_t *info = &human_items[0]) : CBaseAnimating() {
	ChangeTeam(team);
	
	item_info = info;
	
	active = false;

	last_dmg_sprite = 0.0f;
	
	dmg_sprite = NULL;
	gib1 = NULL;
	gib2 = NULL;
	gib3 = NULL;
	gib4 = NULL;
	gib5 = NULL;

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
	Vector origin;
	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;
	
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

	GetTeam()->spend_points(item_info->value);

	active = false;

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
				if (item_info->armored == 1) {
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

//
// Notable ammo/dmg types:
// mech = DMG_SHOCK
// exterm = DMG_PLASMA
// 357 = BULLET & ALWAYSGIB
//
int CMateriel::OnTakeDamage(const CTakeDamageInfo &info) {
	int ret;
	int health;
	int dmgtype;
	float hull_height;
	CHL2MP_Player *p;
	CTakeDamageInfo newinfo = info;

	ret = 0;
	dmgtype = info.GetDamageType();
	p = NULL;

	if (GetTeamNumber() == TEAM_SPIDERS) {

		if (item_info->armored == 0) {
			if (dmgtype & DMG_BLAST ||
				dmgtype & DMG_PLASMA ||
				dmgtype & DMG_SHOCK ||
				dmgtype & DMG_BURN ||
				dmgtype & DMG_CLUB ||
				dmgtype & DMG_SLASH ||
				dmgtype & DMG_BULLET) {

					if (dmgtype & DMG_NEVERGIB) {
						newinfo.ScaleDamage(0.5f);
					}
					
					ret = BaseClass::OnTakeDamage(newinfo);

			}
		} else {
			//
			// armored spider materiel (eggs)
			//
			if (dmgtype & DMG_BLAST ||
				dmgtype & DMG_SHOCK ||
				(dmgtype & DMG_PLASMA && dmgtype & DMG_ALWAYSGIB) ||
				(dmgtype & DMG_BULLET && dmgtype & DMG_ALWAYSGIB)) {

					ret = BaseClass::OnTakeDamage(info);

			}
		}

	} else {

		if (item_info->armored == 0) {
			if (dmgtype & DMG_BLAST ||
				dmgtype & DMG_ACID ||
				dmgtype & DMG_BURN ||
				dmgtype & DMG_CLUB ||
				dmgtype & DMG_SLASH ||
				dmgtype & DMG_SHOCK ||
				dmgtype & DMG_BULLET ||
				(dmgtype & DMG_BULLET && dmgtype & DMG_ALWAYSGIB)) {
					
					if (dmgtype & DMG_NEVERGIB ||
						dmgtype & DMG_BURN ||
						dmgtype & DMG_ACID) {

						// human structures less vulnerable to acid, burn, and weak stuff
						newinfo.ScaleDamage(0.5f);
					}

					ret = BaseClass::OnTakeDamage(newinfo);
			}
		} else {
			//
			// armored human materiel (teleporters)
			//
			if (dmgtype & DMG_BLAST ||
				(dmgtype & DMG_SLASH && dmgtype & DMG_ALWAYSGIB) ||
				(dmgtype & DMG_BULLET && dmgtype & DMG_ALWAYSGIB)) {

					if (dmgtype & DMG_BLAST) {
						newinfo.ScaleDamage(1.3f);
					}

					ret = BaseClass::OnTakeDamage(newinfo);

			} else if (dmgtype & DMG_CLUB || dmgtype & DMG_SLASH) {
				if (info.GetAttacker() && 
					info.GetAttacker()->GetTeamNumber() == TEAM_SPIDERS) {

					if ((p = dynamic_cast<CHL2MP_Player *>(info.GetAttacker())) != NULL) {
						// only larger bugs can slash damage armored human materiel
						if (p->m_iClassNumber == CLASS_DRONE_IDX || p->m_iClassNumber >= CLASS_STINGER_IDX) {
							ret = BaseClass::OnTakeDamage(info);
						}
					}

				}
			}
		}

	}

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