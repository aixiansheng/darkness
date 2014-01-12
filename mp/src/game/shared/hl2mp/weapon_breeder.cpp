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
#define BREEDER_DESTROY_WAIT 3.2f
#define BREEDER_DESTROY_BEEP 1.0f
#define BREEDER_WARN_TIME	1.0f
#define BREEDER_ITEM_CREATION_INTERVAL 1.5f
#define BREEDER_MENU_INTERVAL 0.5f

#define ITEM_UPDATE_INTERVAL	0.3f


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBreeder, DT_WeaponBreeder )

BEGIN_NETWORK_TABLE( CWeaponBreeder, DT_WeaponBreeder )
#ifdef CLIENT_DLL
	RecvPropTime(RECVINFO(m_flNextDestroyBeep)),
#else
	SendPropTime(SENDINFO(m_flNextDestroyBeep)),
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
	m_flNextItemStatus = 0.0f;
	destroy_state = STATE_OFF;
	m_flNextDestroyBeep = FLT_MAX;
}

float CWeaponBreeder::GetDamageForActivity( Activity hitActivity ) {
	return BREEDER_DAMAGE;
}

bool CWeaponBreeder::Deploy(void) {
	bool ret;
	
	m_flNextItemStatus = 0.0f;
	m_flNextDestroyBeep = FLT_MAX;

	ret = BaseClass::Deploy();

	invisible = true;
	destroy_state = STATE_OFF;

	return ret;
}

void CWeaponBreeder::Drop( const Vector &vecVelocity ) {
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
#endif

void CWeaponBreeder::ItemPostFrame( void ) {
	Vector start;
	Vector end;
	Vector dir;
	trace_t tr;
	CBasePlayer *p;
	
	p = ToBasePlayer(GetOwner());
	if (!p)
		return;

#ifndef CLIENT_DLL
	int health;
	int armor;
	CBaseEntity *ent;
	CHL2MP_Player *other;
	CSpiderMateriel *mat;


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

	if ((p->m_nButtons & IN_ATTACK2) && m_flNextSecondaryAttack < gpGlobals->curtime) {
		m_flNextSecondaryAttack = gpGlobals->curtime + BREEDER_REFIRE;
		SecondaryAttack();
	} else {

		switch (destroy_state) {
			//
			// Consider moving UTIL_Trace here:
			// - destroy beep sound would only play when something was actually being destroyed
			// - player couldn't charge destroy unless he/she was actually focused on an object
			//
		case STATE_OFF:
			if ((p->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack < gpGlobals->curtime) {

				destroy_state = STATE_CHARGING;
				m_flNextPrimaryAttack = gpGlobals->curtime + BREEDER_REFIRE;
				m_flDestroyTime = gpGlobals->curtime + BREEDER_DESTROY_WAIT;
				m_flNextDestroyBeep = gpGlobals->curtime + BREEDER_DESTROY_BEEP;
#ifndef CLIENT_DLL
				WarnPlayer();
#endif
			}

			break;

		case STATE_CHARGING:
			if ((p->m_nButtons & IN_ATTACK)) {
				if (m_flDestroyTime < gpGlobals->curtime) {
					destroy_state = STATE_OFF;
					PrimaryAttack();
					m_flNextDestroyBeep = FLT_MAX;
				} else {
					if (m_flNextDestroyBeep < gpGlobals->curtime) {
						WeaponSound(SPECIAL1);
						m_flNextDestroyBeep = gpGlobals->curtime + BREEDER_DESTROY_BEEP;
#ifndef CLIENT_DLL
				WarnPlayer();
#endif
					}
				}
			} else {
				destroy_state = STATE_OFF;
				m_flNextDestroyBeep = FLT_MAX;
			}

			break;

		default:
			break;
		}
	}

	// can't call BaseClass
	// it will just call PrimaryFire...
	// BaseClass::ItemPostFrame();
}

void CWeaponBreeder::SecondaryAttack(void) {
	
	m_flNextSecondaryAttack = gpGlobals->curtime + BREEDER_MENU_INTERVAL;

#ifndef CLIENT_DLL
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(GetOwner());
	if (p) {
		p->ShowViewPortPanel(PANEL_BUILD, true, NULL);
	}

#endif
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

#endif

	CHL2MP_Player *p;
	QAngle stick_ang;
	QAngle turned;
	Vector dir, start, end, fwd, dst;
	trace_t tr;
	int cost;

	m_flNextItemCreation = gpGlobals->curtime + BREEDER_ITEM_CREATION_INTERVAL;

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

	if (p->GetTeamNumber() == TEAM_SPIDERS) {
		switch(idx) {
		case ITEM_EGG_IDX:

			//
			// trace largest unit's hull to avoid stuck spawns
			//
			UTIL_TraceHull(dst, dst, dk_spider_classes[CLASS_STALKER_IDX].vectors->m_vHullMin, dk_spider_classes[CLASS_STALKER_IDX].vectors->m_vHullMax, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}

			cost = spider_items[ITEM_EGG_IDX].value;

			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(spider_items[ITEM_EGG_IDX].ent_name);
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
				}

				#endif
				WeaponSound(SINGLE);
			}

			break;

		case ITEM_HEALER_IDX:

			UTIL_TraceHull(dst, dst, HEALER_HULL_MIN, HEALER_HULL_MAX, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}

			cost = spider_items[ITEM_HEALER_IDX].value;

			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(spider_items[ITEM_HEALER_IDX].ent_name);
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

				#endif
				WeaponSound(SINGLE);
			}
			

			break;

		case ITEM_OBSTACLE_IDX:

			UTIL_TraceHull(dst - (fwd * 20), dst - (fwd * 20), OBSTACLE_HULL_MIN, OBSTACLE_HULL_MAX, MASK_SHOT, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}

			cost = spider_items[ITEM_OBSTACLE_IDX].value;

			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(spider_items[ITEM_OBSTACLE_IDX].ent_name);
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
					obs->SetAbsOrigin(dst - (fwd * 20));
					obs->SetAbsAngles(ang);
					obs->SetCreator(p);
					DispatchSpawn(obs);
				}

				#endif
				WeaponSound(SINGLE);
			}

			break;

		case ITEM_SPIKER_IDX:

			UTIL_TraceHull(dst - (fwd * 20), dst - (fwd * 20), SPIKER_HULL_MIN, SPIKER_HULL_MAX, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}

			cost = spider_items[ITEM_SPIKER_IDX].value;

			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(spider_items[ITEM_SPIKER_IDX].ent_name);
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

				#endif
				WeaponSound(SINGLE);
			}
			

			break;


		case ITEM_GASSER_IDX:

			UTIL_TraceHull(dst, dst, GASSER_HULL_MIN, GASSER_HULL_MAX, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}

			cost = spider_items[ITEM_GASSER_IDX].value;

			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(spider_items[ITEM_GASSER_IDX].ent_name);
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

				#endif
				WeaponSound(SINGLE);
			}
			
			break;
		
		default:
			break;
		}
	}
}

//
// primary attack for breeder
// is for destruction of entities.
// It must be held down...
//
void CWeaponBreeder::PrimaryAttack(void) {
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
				team->reclaim_points(spider_items[ITEM_EGG_IDX].value);
				UTIL_Remove(egg);
			} else if ((healer = dynamic_cast<CHealerEntity *>(ent)) != NULL) {
				team->reclaim_points(spider_items[ITEM_HEALER_IDX].value);
				UTIL_Remove(healer);
			} else if ((obs = dynamic_cast<CObstacleEntity *>(ent)) != NULL) {
				team->reclaim_points(spider_items[ITEM_OBSTACLE_IDX].value);
				UTIL_Remove(obs);
			} else if ((spiker = dynamic_cast<CSpikerEntity *>(ent)) != NULL) {
				team->reclaim_points(spider_items[ITEM_SPIKER_IDX].value);
				UTIL_Remove(spiker);
			} else if ((gasser = dynamic_cast<CGasserEntity *>(ent)) != NULL) {
				team->reclaim_points(spider_items[ITEM_GASSER_IDX].value);
				UTIL_Remove(gasser);
			}
		}
	}

#endif

	//WeaponSound(SINGLE);
	//m_flNextPrimaryAttack = gpGlobals->curtime + BREEDER_REFIRE;
	
	//BaseClass::PrimaryAttack();
}

#ifndef CLIENT_DLL

void CWeaponBreeder::WarnPlayer(void) {
	char buf[64];
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(GetOwner());
	if (p) {
		Q_snprintf(buf, sizeof(buf), "Destroying entity in %.0f seconds!\n", m_flDestroyTime - gpGlobals->curtime);
		UTIL_SayText(buf, p);
	}
}

#endif
