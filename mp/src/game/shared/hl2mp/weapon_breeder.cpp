#include "cbase.h"
#include "hl2mp/weapon_breeder.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "item_info.h"
#include "class_info.h"

#include "ents/egg.h"
#include "ents/healer.h"
#include "ents/obstacle.h"
#include "ents/spiker.h"
#include "ents/gasser.h"
#include "viewport_panel_names.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
	#include "c_team.h"
#else
	#include "hl2mp_player.h"
	#include "gameinterface.h"
	#include "team.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	BREEDER_RANGE	85.0f
#define	BREEDER_REFIRE	2.0f
#define BREEDER_DAMAGE	1.0f
#define BREEDER_WELDER_RANGE 85.0f

#define BREEDER_DESTROY_BEEP 1.8f
#define BREEDER_DESTROY_WAIT (BREEDER_DESTROY_BEEP * 2.95f)

#define BREEDER_DIGEST_SND_TIME 1.3f

#define EGG_TIMER 30.0f
#define EGG_WAIT_FX_TIME 1.8f

#define DEFAULT_HATCH_TIMER 3.0f

#define BREEDER_BITE_DMG 350.0f

#define DIGESTION_TIME 10.0f
#define EGG_DIGESTION_TIME 15.0f
#define HUMAN_DIGESTION_TIME 20.0f

#define ITEM_UPDATE_INTERVAL	0.8f


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBreeder, DT_WeaponBreeder )

BEGIN_NETWORK_TABLE( CWeaponBreeder, DT_WeaponBreeder )
#ifdef CLIENT_DLL
	RecvPropTime(RECVINFO(m_flNextHatchTime)),
	RecvPropTime(RECVINFO(m_flNextFXTime)),
	RecvPropInt(RECVINFO(hatch_state)),
#else
	SendPropTime(SENDINFO(m_flNextHatchTime)),
	SendPropTime(SENDINFO(m_flNextFXTime)),
	SendPropInt(SENDINFO(hatch_state), 3, SPROP_UNSIGNED),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBreeder )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_breeder, CWeaponBreeder );
PRECACHE_WEAPON_REGISTER( weapon_breeder );

acttable_t	CWeaponBreeder::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_PHYSGUN,					false },
};

IMPLEMENT_ACTTABLE(CWeaponBreeder);

CWeaponBreeder::CWeaponBreeder( void ) {
	invisible = true;
	hatch_state = STATE_OFF;
	m_flNextItemStatus = 0.0f;
	m_flNextHatchTime = 0.0f;
	m_flNextFXTime = 0.0f;
	m_flNextPrimaryAttack = 0.0f;
	m_flNextSecondaryAttack = 0.0f;
}

float CWeaponBreeder::GetDamageForActivity( Activity hitActivity ) {
	return BREEDER_DAMAGE;
}

bool CWeaponBreeder::Deploy(void) {
	bool ret;
	
	m_flNextItemStatus = 0.0f;
	m_flNextHatchTime = 0.0f;
	m_flNextFXTime = 0.0f;
	m_flNextPrimaryAttack = 0.0f;
	m_flNextSecondaryAttack = 0.0f;

	ret = BaseClass::Deploy();

	invisible = true;
	hatch_state = STATE_OFF;

	return ret;
}

void CWeaponBreeder::Drop(const Vector &vecVelocity) {
}

float CWeaponBreeder::GetRange(void) {
	return	BREEDER_RANGE;	
}

float CWeaponBreeder::GetFireRate(void) {
	return	BREEDER_REFIRE;	
}

#ifndef CLIENT_DLL

void CWeaponBreeder::ItemStatusUpdate(CBasePlayer *player, int health, int armor) {
	CSingleUserRecipientFilter user(player);
	UserMessageBegin(user, "ItemInfo");
		WRITE_SHORT(armor);
		WRITE_SHORT(health);
	MessageEnd();
}

void CWeaponBreeder::Precache(void) {
	BaseClass::Precache();
}
#endif

void CWeaponBreeder::StartDestroyCharge(void) {
	m_flNextHatchTime = gpGlobals->curtime + BREEDER_DESTROY_WAIT;
	m_flNextFXTime = gpGlobals->curtime;
}

void CWeaponBreeder::HandleItemUpdate(void) {
#ifndef CLIENT_DLL
	int health;
	int armor;
	CBaseEntity *ent;
	CHL2MP_Player *other;
	CSpiderMateriel *mat;
	Vector start;
	Vector end;
	Vector dir;
	trace_t tr;
	CBasePlayer *p;
	
	p = ToBasePlayer(GetOwner());
	if (!p)
		return;

	if (m_flNextItemStatus < gpGlobals->curtime) {
		m_flNextItemStatus = gpGlobals->curtime + ITEM_UPDATE_INTERVAL;

		dir = p->GetAutoaimVector(0);
		start = p->Weapon_ShootPosition();
		end = start + (dir * BREEDER_WELDER_RANGE * 4);

		UTIL_TraceLine(start, end, MASK_ALL, p, COLLISION_GROUP_NONE, &tr);

		ent = tr.m_pEnt;
		armor = -1;
		health = -1;

		if (ent && ent->GetTeamNumber() == TEAM_SPIDERS) {
			if ((other = dynamic_cast<CHL2MP_Player *>(ent)) != NULL) {
				health = other->GetHealth();
				armor = other->ArmorValue();
			} else if ((mat = dynamic_cast<CSpiderMateriel *>(ent)) != NULL) {
				health = mat->GetHealth();
				
				//
				// since breeders can't heal manually, assign creatorship
				// by just looking at entities that have stale creators
				//

				other = mat->GetCreator();
				if (other == NULL || other->GetTeamNumber() != GetTeamNumber()) {
					mat->SetCreator(ToHL2MPPlayer(GetOwner()));
				}
			}
		}

		ItemStatusUpdate(p, health, armor);

	} // m_flNextItemStatus < curtime
#endif
}

void CWeaponBreeder::ItemPostFrame( void ) {
	CBasePlayer *p;
	
	p = ToBasePlayer(GetOwner());
	if (!p)
		return;

	//
	// handle display of health/status of objects in front of
	// breeder/under the crosshair
	//
	HandleItemUpdate();

	//
	// previous section was just for displaying item health
	// all other postframe stuff goes here...
	//

	switch (hatch_state) {
	case STATE_OFF:
		if (p->m_nButtons & IN_ATTACK && m_flNextPrimaryAttack < gpGlobals->curtime) {
			m_flNextPrimaryAttack = gpGlobals->curtime + BREEDER_REFIRE;
			StartDestroyCharge();
			hatch_state = STATE_CHARGING_DESTROY;
		} else if (p->m_nButtons & IN_ATTACK2 && m_flNextSecondaryAttack < gpGlobals->curtime) {
			m_flNextSecondaryAttack = gpGlobals->curtime + BREEDER_REFIRE;
			hatch_state = BreederSecondaryAttack();
			//
			// usually returns STATE_OFF since MakeItem callbacks
			// can only create items from there
			//
		}

		break;

	case STATE_WAITING_EGG:
		if (m_flNextHatchTime < gpGlobals->curtime) {
			WeaponSound(MELEE_MISS);
			hatch_state = STATE_OFF;
		} else if (m_flNextFXTime < gpGlobals->curtime) {
			m_flNextFXTime = gpGlobals->curtime + EGG_WAIT_FX_TIME;
			WeaponSound(SPECIAL1);
		}

		break;

	case STATE_WAITING_NORMAL:
		if (m_flNextHatchTime < gpGlobals->curtime) {
			hatch_state = STATE_OFF;
		}

		break;

	case STATE_CHARGING_DESTROY:
		if (p->m_nButtons & IN_ATTACK) {
			if (m_flNextHatchTime < gpGlobals->curtime) {
				hatch_state = BreederPrimaryAttack();
			} else if (m_flNextFXTime < gpGlobals->curtime) {
				m_flNextFXTime = gpGlobals->curtime + BREEDER_DESTROY_BEEP;
				WeaponSound(RELOAD);
			}
		} else {
			hatch_state = STATE_OFF;
		}

		break;

	case STATE_DIGESTING:
		if (m_flNextHatchTime < gpGlobals->curtime) {
			WeaponSound(MELEE_HIT);
			hatch_state = STATE_OFF;
			m_flNextPrimaryAttack = gpGlobals->curtime + (BREEDER_REFIRE / 2.0f);
		} else if (m_flNextFXTime < gpGlobals->curtime) {
			m_flNextFXTime = gpGlobals->curtime + BREEDER_DIGEST_SND_TIME;
			WeaponSound(SINGLE);
		}

		break;

	default:
		Warning("bad hatch state\n");
		hatch_state = STATE_OFF;
	}

	// can't call BaseClass
	// it will just call PrimaryFire...
	// BaseClass::ItemPostFrame();
}

enum hatch_state CWeaponBreeder::BreederSecondaryAttack(void) {

#ifndef CLIENT_DLL
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(GetOwner());
	if (p) {
		p->ShowViewPortPanel(PANEL_BUILD, true, NULL);
	}

#endif

	return STATE_OFF;
}

int foo = 0;

//
// Use Secondary o show build menu
//
void CWeaponBreeder::MakeItem(int idx) {

	#ifndef CLIENT_DLL

	CBaseEntity *ent = NULL;
	CEggEntity *egg = NULL;
	CHealerEntity *healer = NULL;
	CObstacleEntity *obs = NULL;
	CSpikerEntity *spiker = NULL;
	CGasserEntity *gasser = NULL;
	color32 red = { 255, 0, 0, 80 };
	color32 light_red = { 255, 0, 0, 30 };

	#endif

	CHL2MP_Player *p;
	QAngle stick_ang;
	QAngle turned;
	Vector dir, start, end, fwd, dst;
	trace_t tr;
	int cost;

	if (hatch_state != STATE_OFF)
		return;

	p = ToHL2MPPlayer(GetOwner());
	if (!p) {
		return;
	}
	
	AngleVectors(p->EyeAngles(), &fwd);
	fwd.z = 0;
	VectorNormalize(fwd);

	VectorAngles(fwd, turned);

	dst = p->GetAbsOrigin() + (fwd * 60) + Vector(0, 0, 30);
	QAngle ang(0, p->GetAbsAngles().y - 90, 0);

	hatch_state = STATE_WAITING_NORMAL;

	if (m_flNextHatchTime < gpGlobals->curtime) {
		
		m_flNextHatchTime = gpGlobals->curtime + DEFAULT_HATCH_TIMER;

		if (p->GetTeamNumber() == TEAM_SPIDERS) {
			switch(idx) {
			case ITEM_EGG_IDX:

				//
				// trace largest unit's hull to avoid stuck spawns
				//
				UTIL_TraceHull
				(
					dst,
					dst,
					dk_classes[CLASS_STALKER_IDX].vectors->m_vHullMin,
					dk_classes[CLASS_STALKER_IDX].vectors->m_vHullMax,
					MASK_SOLID,
					NULL,
					&tr
				);

				if (tr.DidHit()) {
					WeaponSound(EMPTY);
					break;
				}

				cost = dk_items[ITEM_EGG_IDX].value;

				if (cost < p->GetTeam()->asset_points) {
					#ifndef CLIENT_DLL
					ent = CreateEntityByName(dk_items[ITEM_EGG_IDX].ent_name);
					if (ent) {
						egg = dynamic_cast<CEggEntity *>(ent);
						if (!egg) {
							UTIL_Remove(ent);
							return;
						}

						egg->SetAbsOrigin(dst);
						egg->SetAbsAngles(ang);
						egg->SetCreator(p);
						DispatchSpawn(egg);

						m_flNextHatchTime = gpGlobals->curtime + EGG_TIMER;
						p->SpeedPenalty(EGG_TIMER);
					}

					UTIL_ScreenFade(p, red, 1.0f, EGG_TIMER, FFADE_MODULATE);

					#endif

					WeaponSound(SPECIAL2);

					hatch_state = STATE_WAITING_EGG;
				}

				break;

			case ITEM_HEALER_IDX:

				UTIL_TraceHull(dst, dst, HEALER_HULL_MIN, HEALER_HULL_MAX, MASK_SOLID, NULL, &tr);
				if (tr.DidHit()) {
					WeaponSound(EMPTY);
					break;
				}

				cost = dk_items[ITEM_HEALER_IDX].value;

				if (cost < p->GetTeam()->asset_points) {
					#ifndef CLIENT_DLL
					ent = CreateEntityByName(dk_items[ITEM_HEALER_IDX].ent_name);
					if (ent) {
						healer = dynamic_cast<CHealerEntity *>(ent);
						if (!healer) {
							UTIL_Remove(ent);
							return;
						}

						healer->SetAbsOrigin(dst);
						healer->SetAbsAngles(ang);
						healer->SetCreator(p);
						DispatchSpawn(healer);
					}

					UTIL_ScreenFade(p, light_red, 0.5f, DEFAULT_HATCH_TIMER, FFADE_MODULATE);

					#endif
				
				}
			

				break;

			case ITEM_OBSTACLE_IDX:

				UTIL_TraceHull
				(
					dst - (fwd * 25),
					dst - (fwd * 25),
					OBSTACLE_HULL_MIN,
					OBSTACLE_HULL_MAX,
					MASK_SHOT,
					NULL,
					&tr
				);

				if (tr.DidHit()) {
					WeaponSound(EMPTY);
					break;
				}

				cost = dk_items[ITEM_OBSTACLE_IDX].value;

				if (cost < p->GetTeam()->asset_points) {
					#ifndef CLIENT_DLL
					ent = CreateEntityByName(dk_items[ITEM_OBSTACLE_IDX].ent_name);
					if (ent) {
						obs = dynamic_cast<CObstacleEntity *>(ent);
						if (!obs) {
							UTIL_Remove(ent);
							return;
						}

						//
						// breeder might use obs/spiker for stacking/climbing, bring it in closer
						// to make climbing easier
						//
						obs->SetAbsOrigin(dst - (fwd * 25));
						obs->SetAbsAngles(ang);
						obs->SetCreator(p);
						DispatchSpawn(obs);
					}

					UTIL_ScreenFade(p, light_red, 0.5f, DEFAULT_HATCH_TIMER, FFADE_MODULATE);

					#endif
				
				}

				break;

			case ITEM_SPIKER_IDX:

				UTIL_TraceHull(dst - (fwd * 20), dst - (fwd * 20), SPIKER_HULL_MIN, SPIKER_HULL_MAX, MASK_SOLID, NULL, &tr);
				if (tr.DidHit()) {
					WeaponSound(EMPTY);
					break;
				}

				cost = dk_items[ITEM_SPIKER_IDX].value;

				if (cost < p->GetTeam()->asset_points) {
					#ifndef CLIENT_DLL
					ent = CreateEntityByName(dk_items[ITEM_SPIKER_IDX].ent_name);
					if (ent) {
						spiker = dynamic_cast<CSpikerEntity *>(ent);
						if (!spiker) {
							UTIL_Remove(ent);
							return;
						}

						//
						// breeder might use obs/spiker for stacking/climbing, bring it in closer
						// to make climbing easier
						//
						spiker->SetAbsOrigin(dst - (fwd * 20));
						spiker->SetAbsAngles(ang);
						spiker->SetCreator(p);
						DispatchSpawn(spiker);
					}

					UTIL_ScreenFade(p, light_red, 0.5f, DEFAULT_HATCH_TIMER, FFADE_MODULATE);

					#endif
				
				}
			

				break;


			case ITEM_GASSER_IDX:

				UTIL_TraceHull(dst, dst, GASSER_HULL_MIN, GASSER_HULL_MAX, MASK_SOLID, NULL, &tr);
				if (tr.DidHit()) {
					WeaponSound(EMPTY);
					break;
				}

				cost = dk_items[ITEM_GASSER_IDX].value;

				if (cost < p->GetTeam()->asset_points) {
					#ifndef CLIENT_DLL
					ent = CreateEntityByName(dk_items[ITEM_GASSER_IDX].ent_name);
					if (ent) {
						gasser = dynamic_cast<CGasserEntity *>(ent);
						if (!gasser) {
							UTIL_Remove(ent);
							return;
						}

						gasser->SetAbsOrigin(dst);
						gasser->SetAbsAngles(turned);
						gasser->SetCreator(p);
						DispatchSpawn(gasser);
					}

					UTIL_ScreenFade(p, light_red, 0.5f, DEFAULT_HATCH_TIMER, FFADE_MODULATE);

					#endif
				
				}
			
				break;
		
			default:
				hatch_state = STATE_OFF;
				break;
			} // switch

		} // if (TEAM_SPIDERS)

	} //if (m_flNextHatchTime < gpGlobals->curtime)
}

//
// primary attack for breeder
// is for destruction of entities.
// It must be held down...
//
enum hatch_state CWeaponBreeder::BreederPrimaryAttack(void) {
#ifndef CLIENT_DLL
	Vector start;
	Vector end;
	Vector dir;
	trace_t tr;
	CTeam *team;
	CBasePlayer *p;
	CBaseEntity *ent;
	CEggEntity *egg;
	CHealerEntity *healer;
	CObstacleEntity *obs;
	CSpikerEntity *spiker;
	CGasserEntity *gasser;
	CHL2MP_Player *targetPlayer;
	float biteDmg;

	team = GetTeam();
	p = ToBasePlayer(GetOwner());
	if (p && team) {
		dir = p->GetAutoaimVector(0);
		start = p->Weapon_ShootPosition();
		end = start + (dir * BREEDER_WELDER_RANGE);

		UTIL_TraceLine(start, end, MASK_ALL, p, COLLISION_GROUP_NONE, &tr);

		ent = tr.m_pEnt;

		if (ent) {
			if ((egg = dynamic_cast<CEggEntity*>(ent)) != NULL) {
				team->RemoveSpawnpoint(egg->SpawnPoint());
				team->reclaim_points(dk_items[ITEM_EGG_IDX].value);
				UTIL_Remove(egg);
				m_flNextHatchTime = gpGlobals->curtime + EGG_DIGESTION_TIME;
				m_flNextFXTime = gpGlobals->curtime + BREEDER_DIGEST_SND_TIME;
				return STATE_DIGESTING;
			} else if ((healer = dynamic_cast<CHealerEntity *>(ent)) != NULL) {
				team->reclaim_points(dk_items[ITEM_HEALER_IDX].value);
				UTIL_Remove(healer);
				m_flNextHatchTime = gpGlobals->curtime + DIGESTION_TIME;
				m_flNextFXTime = gpGlobals->curtime + BREEDER_DIGEST_SND_TIME;
				return STATE_DIGESTING;
			} else if ((obs = dynamic_cast<CObstacleEntity *>(ent)) != NULL) {
				team->reclaim_points(dk_items[ITEM_OBSTACLE_IDX].value);
				UTIL_Remove(obs);
				m_flNextHatchTime = gpGlobals->curtime + DIGESTION_TIME;
				m_flNextFXTime = gpGlobals->curtime + BREEDER_DIGEST_SND_TIME;
				return STATE_DIGESTING;
			} else if ((spiker = dynamic_cast<CSpikerEntity *>(ent)) != NULL) {
				team->reclaim_points(dk_items[ITEM_SPIKER_IDX].value);
				UTIL_Remove(spiker);
				m_flNextHatchTime = gpGlobals->curtime + DIGESTION_TIME;
				m_flNextFXTime = gpGlobals->curtime + BREEDER_DIGEST_SND_TIME;
				return STATE_DIGESTING;
			} else if ((gasser = dynamic_cast<CGasserEntity *>(ent)) != NULL) {
				team->reclaim_points(dk_items[ITEM_GASSER_IDX].value);
				UTIL_Remove(gasser);
				m_flNextHatchTime = gpGlobals->curtime + DIGESTION_TIME;
				m_flNextFXTime = gpGlobals->curtime + BREEDER_DIGEST_SND_TIME;
				return STATE_DIGESTING;
			} else if ((targetPlayer = dynamic_cast<CHL2MP_Player *>(ent)) != NULL) {

				//
				// breeder can bite humans
				//
				switch (targetPlayer->m_iClassNumber) {
				case CLASS_ENGINEER_IDX:
				case CLASS_GRUNT_IDX:
				case CLASS_SHOCK_IDX:
				case CLASS_HEAVY_IDX:
				case CLASS_COMMANDO_IDX:
					biteDmg = BREEDER_BITE_DMG;
					break;

				case CLASS_HATCHY_IDX:
				case CLASS_DRONE_IDX:
				case CLASS_KAMI_IDX:
					biteDmg = BREEDER_BITE_DMG / 4.0f;
					break;

				default:
					// breeders, big bugs, mechs, exterms
					biteDmg = 0.0f;
					break;
				}

				// if the player is eaten, obviously there should be no ragdoll
				targetPlayer->ForceNoRagdoll(true);

				CTakeDamageInfo info(this, p, biteDmg, DMG_CLUB);
				CalculateMeleeDamageForce(&info, dir, start, 5.0f);
				targetPlayer->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();

				if (!targetPlayer->IsAlive()) {
					m_flNextHatchTime = gpGlobals->curtime + HUMAN_DIGESTION_TIME;
					m_flNextFXTime = gpGlobals->curtime + BREEDER_DIGEST_SND_TIME;
					return STATE_DIGESTING;
				} else {
					// player can ragdoll for someone else if they survived the bite
					targetPlayer->ForceNoRagdoll(false);
				}
			}
		}
	}

#endif
	WeaponSound(BURST);
	return STATE_OFF;
}
