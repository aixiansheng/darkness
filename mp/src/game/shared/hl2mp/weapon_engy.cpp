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
	#include "ents/teleporter.h"
	#include "ents/ammo_crate.h"
	#include "ents/egg.h"
	#include "ents/medipad.h"
	#include "ents/exp_mine.h"
	#include "ents/mg_turret.h"
	#include "ents/missile_turret.h"
	#include "ents/detector.h"
	#include "gameinterface.h"
	#include "team.h"
	#include "particle_parse.h"
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
	CBaseEntity *ent = NULL;
	CTeleporterEntity *tele = NULL;
	CAmmoCrate *ammo = NULL;
	CMedipadEntity *medipad = NULL;
	CExpMineEntity *mine = NULL;
	CMGTurretEntity *turret = NULL;
	CDetectorEntity *det = NULL;
	CMSLTurretEntity *missile_turret = NULL;
#endif

	int cost;
	CHL2MP_Player *p;
	QAngle stick_ang;
	QAngle turned;
	Vector dir, start, end, fwd, dst;
	trace_t tr;

	m_flNextItemCreation = gpGlobals->curtime + ENGY_CREATION_INTERVAL;

	p = ToHL2MPPlayer(GetOwner());
	if (!p) {
		return;
	}
	
	AngleVectors(p->EyeAngles(), &fwd);
	fwd.z = 0;
	VectorNormalize(fwd);
	VectorAngles(fwd, turned);

	dst = p->GetAbsOrigin() + (fwd * 60) + Vector(0, 0, 60);
	QAngle ang(0, p->GetAbsAngles().y - 90, 0);

	if (p->GetTeamNumber() == TEAM_HUMANS) {
		switch(idx) {
		case ITEM_TELEPORTER_IDX:

			//
			// trace the largest unit's hull size to prevent stuck spawns
			//
			UTIL_TraceHull(dst, dst, dk_human_classes[CLASS_MECH_IDX].vectors->m_vHullMin, dk_human_classes[CLASS_MECH_IDX].vectors->m_vHullMax, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}


			cost = human_items[ITEM_TELEPORTER_IDX].value;
			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(human_items[ITEM_TELEPORTER_IDX].ent_name);
				if (ent) {
					tele = dynamic_cast<CTeleporterEntity *>(ent);
					if (!tele) {
						UTIL_Remove(ent);
						return;
					}

					tele->SetAbsOrigin(dst);
					tele->SetAbsAngles(ang);
					tele->SetCreator(p);
					DispatchSpawn(tele);
					tele->Disable();
				}
				#endif

				WeaponSound(SINGLE);
			}


			break;
		
		case ITEM_AMMO_CRATE_IDX:
			UTIL_TraceHull(dst, dst, AMMO_CRATE_HULL_MIN, AMMO_CRATE_HULL_MAX, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}


			cost = human_items[ITEM_AMMO_CRATE_IDX].value;
			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(human_items[ITEM_AMMO_CRATE_IDX].ent_name);
				if (ent) {
					ammo = dynamic_cast<CAmmoCrate *>(ent);
					if (!ammo) {
						UTIL_Remove(ent);
						return;
					}

					ammo->SetAbsOrigin(dst);
					ammo->SetAbsAngles(ang);
					ammo->SetCreator(p);
					DispatchSpawn(ammo);
				}
				#endif

				WeaponSound(SINGLE);
			}

			break;
		
		case ITEM_MEDIPAD_IDX:
			UTIL_TraceHull(dst, dst, MEDIPAD_HULL_MIN, MEDIPAD_HULL_MAX, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}


			cost = human_items[ITEM_MEDIPAD_IDX].value;
			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(human_items[ITEM_MEDIPAD_IDX].ent_name);
				if (ent) {
					medipad = dynamic_cast<CMedipadEntity *>(ent);
					if (!medipad) {
						UTIL_Remove(ent);
						return;
					}

					medipad->SetAbsOrigin(dst);
					medipad->SetAbsAngles(ang);
					medipad->SetCreator(p);
					DispatchSpawn(medipad);
				}
				#endif

				WeaponSound(SINGLE);
			}

			break;

		case ITEM_MINE_IDX:
			dir = ToBasePlayer(p)->GetAutoaimVector(0);
			start = p->Weapon_ShootPosition();
			end = start + (dir * ENGY_WELDER_RANGE);

			UTIL_TraceLine(start, end, MASK_ALL, p, COLLISION_GROUP_NONE, &tr);

			if (tr.fraction == 1.0f) {
				WeaponSound(EMPTY);
				break;
			}


			cost = human_items[ITEM_MINE_IDX].value;

			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(human_items[ITEM_MINE_IDX].ent_name);
				if (ent) {
					mine = dynamic_cast<CExpMineEntity *>(ent);
					if (!mine) {
						UTIL_Remove(ent);
						return;
					}

					VectorAngles(tr.plane.normal, stick_ang);
					stick_ang.y -= 50;
					//stick_ang.z -= 90;
					//tr.endpos.z -= 6.0f;

					mine->SetAbsOrigin(tr.endpos + tr.plane.normal * 3);
					mine->SetAbsAngles(stick_ang);
					mine->SetCreator(p);
					DispatchSpawn(mine);
				}
				#endif
			}


			break;

		case ITEM_DETECTOR_IDX:
			dir = ToBasePlayer(p)->GetAutoaimVector(0);
			start = p->Weapon_ShootPosition();
			end = start + (dir * ENGY_WELDER_RANGE);

			UTIL_TraceLine(start, end, MASK_ALL, p, COLLISION_GROUP_NONE, &tr);

			if (tr.fraction == 1.0f) {
				WeaponSound(EMPTY);
				break;
			}

			cost = human_items[ITEM_DETECTOR_IDX].value;

			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(human_items[ITEM_DETECTOR_IDX].ent_name);
				if (ent) {
					det = dynamic_cast<CDetectorEntity *>(ent);
					if (!det) {
						UTIL_Remove(ent);
						return;
					}

					VectorAngles(tr.plane.normal, stick_ang);
					stick_ang.y -= 50;
					//stick_ang.z -= 90;
					//tr.endpos.z -= 6.0f;

					det->SetAbsOrigin(tr.endpos + tr.plane.normal * 2);
					det->SetAbsAngles(stick_ang);
					det->SetCreator(p);
					DispatchSpawn(det);
				}
				#endif
			}


			break;

		case ITEM_SMG_TURRET_IDX:
			UTIL_TraceHull(dst, dst, SMG_TURRET_HULL_MIN, SMG_TURRET_HULL_MAX, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}

			cost = human_items[ITEM_SMG_TURRET_IDX].value;
			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(human_items[ITEM_SMG_TURRET_IDX].ent_name);
				if (ent) {
					turret = dynamic_cast<CMGTurretEntity *>(ent);
					if (!turret) {
						UTIL_Remove(turret);
						return;
					}

					turret->SetAbsOrigin(dst);
					turret->SetAbsAngles(turned);
					turret->SetCreator(p);
					DispatchSpawn(turret);
				}
				#endif
			}


			break;

		case ITEM_MSL_TURRET_IDX:
			UTIL_TraceHull(dst, dst, MSL_TURRET_HULL_MIN, MSL_TURRET_HULL_MAX, MASK_SOLID, NULL, &tr);
			if (tr.DidHit()) {
				WeaponSound(EMPTY);
				break;
			}

			cost = human_items[ITEM_MSL_TURRET_IDX].value;
			if (cost < p->GetTeam()->asset_points) {
				#ifndef CLIENT_DLL
				ent = CreateEntityByName(human_items[ITEM_MSL_TURRET_IDX].ent_name);
				if (ent) {
					missile_turret = dynamic_cast<CMSLTurretEntity *>(ent);
					if (!missile_turret) {
						UTIL_Remove(missile_turret);
						return;
					}

					missile_turret->SetAbsOrigin(dst);
					missile_turret->SetAbsAngles(turned);
					missile_turret->SetCreator(p);
					DispatchSpawn(missile_turret);
				}
				#endif
			}


			break;

		default:
			break;
		}
	}
}

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

				} else {
					// damage the ent a little
					ClearMultiDamage();

					UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + dir * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

					CTakeDamageInfo dmg(this, GetOwner(), ENGY_DAMAGE, DMG_BULLET);
					CalculateMeleeDamageForce(&dmg, dir, tr.endpos);
					other->DispatchTraceAttack(dmg, dir, &tr);

					ApplyMultiDamage();
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

void CWeaponEngy::DisplayUsageHudHint(void) {
	UTIL_HudHintText(GetOwner(), "%+attack2%  show build menu\n%+attack%  repair");
	BaseClass::DisplayUsageHudHint();
}

void CWeaponEngy::Precache(void) {
	PrecacheParticleSystem(ENGY_FIX_PARTICLE);
	BaseClass::Precache();
}

#endif
