//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Engy - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp/weapon_engy.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "item_info.h"
#include "viewport_panel_names.h"
#include "class_info.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "gameinterface.h"
	#include "team.h"
	#include "particle_parse.h"
	#include "ents/teleporter.h"
	#include "ents/ammo_crate.h"
	#include "ents/egg.h"
	#include "ents/medipad.h"
	#include "ents/exp_mine.h"
	#include "ents/mg_turret.h"
	#include "ents/missile_turret.h"
	#include "ents/detector.h"
#endif



// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	ENGY_RANGE	85.0f
#define	ENGY_REFIRE 0.85f
#define ENGY_DAMAGE	5.0f
#define ENGY_WELDER_RANGE 115.0f
#define ENGY_HEAL_AMT 10.0f
#define ENGY_CREATION_INTERVAL 2.0f
#define ENGY_MENU_INTERVAL	0.5f

#define ITEM_UPDATE_INTERVAL	0.4f

#define ENGY_FIX_PARTICLE "sparking_gear"

//-----------------------------------------------------------------------------
// CWeaponEngy
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponEngy, DT_WeaponEngy )

BEGIN_NETWORK_TABLE( CWeaponEngy, DT_WeaponEngy )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponEngy )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_engy, CWeaponEngy );
PRECACHE_WEAPON_REGISTER( weapon_engy );

acttable_t	CWeaponEngy::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_PHYSGUN,					false },
};

IMPLEMENT_ACTTABLE(CWeaponEngy);

CWeaponEngy::CWeaponEngy( void ) {
	SetWeaponVisible(false);
	AddEffects(EF_NODRAW);
	m_flNextItemStatus = 0.0f;
	m_flNextItemCreation = 0.0f;
#ifndef CLIENT_DLL
	repaired_total = 0;
#endif
}

float CWeaponEngy::GetDamageForActivity( Activity hitActivity ) {
	return ENGY_DAMAGE;
}

bool CWeaponEngy::Deploy(void) {
	bool ret;
	
	m_flNextItemStatus = 0.0f;

	ret = BaseClass::Deploy();
	AddEffects(EF_NODRAW);
	return ret;
}

void CWeaponEngy::Drop( const Vector &vecVelocity ) {
}

float CWeaponEngy::GetRange(void) {
	return	ENGY_RANGE;	
}

float CWeaponEngy::GetFireRate(void) {
	return	ENGY_REFIRE;	
}

#ifndef CLIENT_DLL
void CWeaponEngy::ItemStatusUpdate(CBasePlayer *player, int health, int armor) {
	CSingleUserRecipientFilter user(player);
	UserMessageBegin(user, "ItemInfo");
		WRITE_SHORT(armor);
		WRITE_SHORT(health);
	MessageEnd();
}
#endif

void CWeaponEngy::ItemPostFrame( void ) {
#ifndef CLIENT_DLL
	Vector start;
	Vector end;
	Vector dir;
	trace_t tr;
	int health;
	int armor;
	CBasePlayer *p;
	CBaseEntity *ent;
	CHL2MP_Player *other;
	CHumanMateriel *mat;

	if (m_flNextItemStatus < gpGlobals->curtime) {
		m_flNextItemStatus = gpGlobals->curtime + ITEM_UPDATE_INTERVAL;
		p = ToBasePlayer(GetOwner());
		if (p) {
			dir = p->GetAutoaimVector(0);
			start = p->Weapon_ShootPosition();
			end = start + (dir * ENGY_WELDER_RANGE * 4);

			UTIL_TraceLine(start, end, MASK_ALL, this, COLLISION_GROUP_NONE, &tr);

			ent = tr.m_pEnt;

			armor = -1;
			health = -1;

			if (ent && ent->GetTeamNumber() == TEAM_HUMANS) {
				if ((other = dynamic_cast<CHL2MP_Player *>(ent)) != NULL) {
					health = other->GetHealth();
					armor = other->ArmorValue();
				} else if ((mat = dynamic_cast<CHumanMateriel *>(ent)) != NULL) {
					health = mat->GetHealth();
				}
			}

			ItemStatusUpdate(p, health, armor);
		}

	} // m_flNextItemStatus < curtime

#endif

	BaseClass::ItemPostFrame();
}

void CWeaponEngy::MakeItem(int idx) {
#ifndef CLIENT_DLL
	CTeleporterEntity *tele = NULL;
	CExpMineEntity *mine = NULL;
	CHumanMateriel *mat = NULL;

	bool autokill;
	int cost;
	CHL2MP_Player *p;
	QAngle stick_ang;
	QAngle turned;
	Vector dir, start, end, fwd, dst;
	trace_t tr;
	Vector normal;

	m_flNextItemCreation = gpGlobals->curtime + ENGY_CREATION_INTERVAL;

	p = ToHL2MPPlayer(GetOwner());
	if (!p)
		return;
	
	AngleVectors(p->EyeAngles(), &fwd);
	fwd.z = 0;
	VectorNormalize(fwd);
	VectorAngles(fwd, turned);

	dst = p->GetAbsOrigin() + (fwd * 60) + Vector(0,0,20);
	QAngle ang(0, p->GetAbsAngles().y - 90, 0);

	if (p->GetTeamNumber() == TEAM_HUMANS) {
		//
		// make sure the item cost is not too high
		//
		switch (idx) {
		case ITEM_TELEPORTER_IDX:
		case ITEM_MEDIPAD_IDX:
		case ITEM_AMMO_CRATE_IDX:
		case ITEM_MINE_IDX:
		case ITEM_DETECTOR_IDX:
		case ITEM_SMG_TURRET_IDX:
		case ITEM_MSL_TURRET_IDX:
			cost = dk_items[idx].value;
			if (cost > p->GetTeam()->asset_points)
				return;
			break;

		default:
			return;
		}

		switch (idx) {

		case ITEM_TELEPORTER_IDX:
			turned = ang;
			mat = SpawnItem(dk_items[idx].ent_name, dst, turned, p, true, false, tr);

			if ((tele = dynamic_cast<CTeleporterEntity *>(mat)) != NULL) {
				//
				// no collision happened, see if the bounds
				// are good for a regular teleporter spawning a mech
				//
				UTIL_TraceHull
				(
					dst,
					dst,
					dk_classes[CLASS_MECH_IDX].vectors->m_vHullMin,
					dk_classes[CLASS_MECH_IDX].vectors->m_vHullMax,
					MASK_SOLID,
					tele,
					COLLISION_GROUP_DEBRIS,
					&tr
				);

				if (tr.DidHit()) {
					tele->RefundPoints();
					UTIL_Remove(tele);
					tele = NULL;
					mat = NULL;
				}

				autokill = false;

			} else {
				//
				// create a teleporter for the engineer to jump on
				//
				dst = dst + Vector(0,0,15);
				mat = SpawnItem(dk_items[idx].ent_name, dst, turned, p, true, true, tr);
				tele = dynamic_cast<CTeleporterEntity *>(mat);

				autokill = true;
			}

			if (tele && autokill) {
				tele->AutoKill();
			}

			break;

		case ITEM_MINE_IDX:
		case ITEM_DETECTOR_IDX:
			if (WallStickParams(p, dst, turned, tr)) {
				normal = tr.plane.normal;
				mat = SpawnItem(dk_items[idx].ent_name, dst, turned, p, false, true, tr);
				if ((mine = dynamic_cast<CExpMineEntity *>(mat)) != NULL) {
					mine->SetNormal(normal);
				}
			}

			break;

		case ITEM_AMMO_CRATE_IDX:
		case ITEM_MEDIPAD_IDX:
			turned = ang;
		case ITEM_SMG_TURRET_IDX:
		case ITEM_MSL_TURRET_IDX:
			mat = SpawnItem(dk_items[idx].ent_name, dst, turned, p, true, false, tr);

			break;

		default:
			break;
		}

		//
		// potentiall do something based on mat being NULL
		// like writing a message to the client, or emitting
		// a sound right here...
		//

		if (mat) {
			mat->DoSpawnSound();
		} else {
			WeaponSound(EMPTY);
		}
	}
#endif
}

#ifndef CLIENT_DLL

bool CWeaponEngy::WallStickParams(CHL2MP_Player *player, Vector &position, QAngle &angles, trace_t &tr) {
	Vector direction;
	Vector start;
	Vector end;

	direction = ToBasePlayer(player)->GetAutoaimVector(0);
	start = player->Weapon_ShootPosition();
	end = start + (direction * ENGY_WELDER_RANGE);

	UTIL_TraceLine(start, end, MASK_ALL, player, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction == 1.0f)
		return false;

	VectorAngles(tr.plane.normal, angles);
	angles.y -= 50;
	position = tr.endpos + tr.plane.normal * 2;

	return true;
}

CHumanMateriel *
CWeaponEngy::SpawnItem(
	const char *name,
	Vector &origin, 
	QAngle &angles,
	CHL2MP_Player *creator,
	bool start_disabled,
	bool ignore_collision,
	trace_t &tr) 
{
	CHumanMateriel *item;

	item = dynamic_cast<CHumanMateriel *>(CreateEntityByName(name));
	if (item == NULL)
		return NULL;

	item->SetAbsOrigin(origin);
	item->SetAbsAngles(angles);
	item->SetCreator(creator);
	DispatchSpawn(item);
	
	if (start_disabled)
		item->Disable();

	UTIL_TraceEntity(item, origin, origin, MASK_SOLID, &tr);
	if (tr.DidHit() && ignore_collision == false) {
		item->RefundPoints();
		UTIL_Remove(item);
		return NULL;
	}

	return item;
}
#endif

//
// Secondary attack makes the build menu appear
//
void CWeaponEngy::SecondaryAttack(void) {
	
	m_flNextSecondaryAttack = gpGlobals->curtime + ENGY_MENU_INTERVAL;

#ifndef CLIENT_DLL
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(GetOwner());
	if (p) {
		p->ShowViewPortPanel(PANEL_BUILD, true, NULL);
	}

#endif
}

void CWeaponEngy::PrimaryAttack(void) {
#ifndef CLIENT_DLL
	Vector start;
	Vector end;
	Vector dir;
	trace_t tr;
	CBasePlayer *p;
	CBaseEntity *ent;
	CHumanMateriel *mat;
	CHL2MP_Player *other;
	int healAmt = ENGY_HEAL_AMT;
	int before, after;

	p = ToBasePlayer(GetOwner());
	if (p) {
		dir = p->GetAutoaimVector(0);
		start = p->Weapon_ShootPosition();
		end = start + (dir * ENGY_WELDER_RANGE);

		UTIL_TraceLine(start, end, MASK_ALL, this, COLLISION_GROUP_NONE, &tr);

		ent = tr.m_pEnt;

		if (ent) {
			if ((mat = dynamic_cast<CHumanMateriel *>(ent)) != NULL) {
				before = mat->GetHealth();
				mat->TakeHealth(healAmt, DMG_GENERIC);
				after = mat->GetHealth();

				//
				// if the original creator has changed teams, or never existed
				// take ownership of this materiel
				//
				other = mat->GetCreator();
				if (other == NULL || other->GetTeamNumber() != GetTeamNumber()) {
					mat->SetCreator(ToHL2MPPlayer(p));
				}

				if (after - before > 0) {
					repaired_total += (after - before);

					if (repaired_total % 300 == 0) {
						other = ToHL2MPPlayer(p);
						if (other) {
							other->IncrementFragCount(1);
						}
					}
				}

			} else if ((other = dynamic_cast<CHL2MP_Player *>(ent)) != NULL) {
				if (other->GetTeamNumber() == TEAM_HUMANS) {
					//
					// don't heal players, just give them armor
					// they should use the ammo depot or medipad for health packs
					//
					before = other->ArmorValue();
					other->IncrementArmorValue(5, other->GetMaxArmor());
					after = other->ArmorValue();

					if (after - before > 0) {
						repaired_total += (after - before);

						if (repaired_total % 300 == 0) {
							other = ToHL2MPPlayer(p);
							if (other) {
								other->IncrementFragCount(1);
							}
						}
					}

					other->RefilAmmo(true);

				}
			}

			CDisablePredictionFiltering foo;
			DispatchParticleEffect(ENGY_FIX_PARTICLE, tr.endpos + Vector(0,0,5), GetAbsAngles(), ent);
		}
	}

#endif

	WeaponSound(SINGLE);
	m_flNextPrimaryAttack = gpGlobals->curtime + ENGY_REFIRE;
	
	//BaseClass::PrimaryAttack();
}



#ifndef CLIENT_DLL

void CWeaponEngy::Precache(void) {
	PrecacheParticleSystem(ENGY_FIX_PARTICLE);
	BaseClass::Precache();
}

#endif
