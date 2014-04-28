//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Engy - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp/weapon_engy_destroy.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "item_info.h"

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
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	ENGY_RANGE	85.0f
#define	ENGY_REFIRE	5.0f
#define ENGY_WELDER_RANGE 115.0f

#define ITEM_UPDATE_INTERVAL	0.5f

#define DESTROY_CHARGE_TIME 7.0f
#define CHARGE_WARN_TIME 2.0f
#define DEFAULT_WARN_BEEPS 3

#define DISCHARGE_OWNER_DMG 35
#define DISCHARGE_PLAYER_DMG 40
#define DISCHARGE_ENT_DMG 30

//-----------------------------------------------------------------------------
// CWeaponEngyDestroy
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponEngyDestroy, DT_WeaponEngyDestroy )

BEGIN_NETWORK_TABLE( CWeaponEngyDestroy, DT_WeaponEngyDestroy )
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(warnBeepsLeft)),
	RecvPropFloat(RECVINFO(nextWarnBeep)),
#else
	SendPropInt(SENDINFO(warnBeepsLeft), 3, SPROP_UNSIGNED),
	SendPropFloat(SENDINFO(nextWarnBeep)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponEngyDestroy )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_engy_destroy, CWeaponEngyDestroy );
PRECACHE_WEAPON_REGISTER( weapon_engy_destroy );

#define	MEGACANNON_MODEL "models/weapons/v_superphyscannon.mdl"
#define	MEGACANNON_SKIN	1

#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponEngyDestroy )
	DEFINE_FUNCTION( ChargeThink ),
	DEFINE_FUNCTION( DrainThink ),
END_DATADESC()
#endif

acttable_t	CWeaponEngyDestroy::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_PHYSGUN,					false },
};

IMPLEMENT_ACTTABLE(CWeaponEngyDestroy);

CWeaponEngyDestroy::CWeaponEngyDestroy( void ) {
	SetWeaponVisible(false);
	AddEffects(EF_NODRAW);
	m_flNextItemStatus = 0.0f;
}

float CWeaponEngyDestroy::GetDamageForActivity( Activity hitActivity ) {
	return 0.0f;
}

bool CWeaponEngyDestroy::Deploy(void) {
	bool ret;
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(GetOwner());
	if (p) {
		p->SetAmmoCount(0, m_iPrimaryAmmoType);
	}
	
	m_flNextItemStatus = 0.0f;

	ret = BaseClass::Deploy();
	AddEffects(EF_NODRAW);
	return ret;
}

void CWeaponEngyDestroy::Drop( const Vector &vecVelocity ) {
}

float CWeaponEngyDestroy::GetRange(void) {
	return	ENGY_RANGE;	
}

float CWeaponEngyDestroy::GetFireRate(void) {
	return	ENGY_REFIRE;	
}

#ifndef CLIENT_DLL
void CWeaponEngyDestroy::ItemStatusUpdate(CBasePlayer *player, int health, int armor) {
	CSingleUserRecipientFilter user(player);
	UserMessageBegin(user, "ItemInfo");
		WRITE_SHORT(armor);
		WRITE_SHORT(health);
	MessageEnd();
}

void CWeaponEngyDestroy::Spawn(void) {
	BaseClass::Spawn();
	SetAmmoType(GetAmmoDef()->Index("engy_destroy"));

	gun_state = STATE_OFF;
}

#endif

void CWeaponEngyDestroy::ItemPostFrame( void ) {

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
	CTeleporterEntity *tele;
	CAmmoCrate *ammo;
	CMedipadEntity *medipad;
	CMGTurretEntity *mgturret;
	CDetectorEntity *det;
	CMSLTurretEntity *missile_turret;

	if (m_flNextItemStatus < gpGlobals->curtime) {
		m_flNextItemStatus = gpGlobals->curtime + ITEM_UPDATE_INTERVAL;
		p = ToBasePlayer(GetOwner());
		if (p) {
			dir = p->GetAutoaimVector(0);
			start = p->Weapon_ShootPosition();
			end = start + (dir * ENGY_WELDER_RANGE * 4);

			UTIL_TraceLine(start, end, MASK_ALL, p, COLLISION_GROUP_NONE, &tr);

			ent = tr.m_pEnt;

			armor = -1;
			health = -1;

			if (ent && ent->GetTeamNumber() == TEAM_HUMANS) {
				if ((other = dynamic_cast<CHL2MP_Player *>(ent)) != NULL) {
					health = other->GetHealth();
					armor = other->ArmorValue();
				} else if ((tele = dynamic_cast<CTeleporterEntity*>(ent)) != NULL) {
					health = tele->GetHealth();
				} else if ((ammo = dynamic_cast<CAmmoCrate *>(ent)) != NULL) {
					health = ammo->GetHealth();
				} else if ((medipad = dynamic_cast<CMedipadEntity *>(ent)) != NULL) {
					health = medipad->GetHealth();
				} else if ((mgturret = dynamic_cast<CMGTurretEntity *>(ent)) != NULL) {
					health = mgturret->GetHealth();
				} else if ((det = dynamic_cast<CDetectorEntity *>(ent)) != NULL) {
					health = det->GetHealth();
				} else if ((missile_turret = dynamic_cast<CMSLTurretEntity *>(ent)) != NULL) {
					health = missile_turret->GetHealth();
				}
			}

			ItemStatusUpdate(p, health, armor);
		}

	} // m_flNextItemStatus < curtime

#endif

	//
	// stuff that happens on client and server
	//
	int charge;
	CBasePlayer *owner;
	CHL2MP_Player *pHL2MPPlayer;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	pHL2MPPlayer = ToHL2MPPlayer(owner);
	if (!pHL2MPPlayer)
		return;

	charge = pHL2MPPlayer->GetAmmoCount(m_iPrimaryAmmoType);

	switch (gun_state) {
	case STATE_OFF:
		if (owner->m_nButtons & IN_ATTACK) {
			pHL2MPPlayer->SetAmmoCount(0, m_iPrimaryAmmoType);
			gun_state = STATE_CHARGING;
			SetThink(&CWeaponEngyDestroy::ChargeThink);
			SetNextThink(gpGlobals->curtime + 0.1f);
			Warning("Charging\n");
		}
		break;

	case STATE_CHARGING:
		if (owner->m_nButtons & IN_ATTACK) {
			if (charge >= 100) {
				gun_state = STATE_CHARGED;
				nextWarnBeep = gpGlobals->curtime + CHARGE_WARN_TIME;
				warnBeepsLeft = DEFAULT_WARN_BEEPS;
				SetThink(NULL);

				// full charge beep
				Warning("Full charge\n");

			}
		} else {
			gun_state = STATE_DRAINING;
			SetThink(&CWeaponEngyDestroy::DrainThink);
			SetNextThink(gpGlobals->curtime + 0.1f);
			Warning("Draining\n");
		}
		break;

	case STATE_CHARGED:
		if (owner->m_nButtons & IN_ATTACK) {
			if (gpGlobals->curtime > nextWarnBeep) {
				if (warnBeepsLeft > 0) {
					nextWarnBeep = gpGlobals->curtime + CHARGE_WARN_TIME;
					warnBeepsLeft--;
					Warning("Warning Beep\n");
				} else {
					gun_state = STATE_OFF;
					SetThink(NULL);
					// cause damage to owner
					// shock sound
					Warning("Shock!\n");
					DamageOwner();
				}
			}
		} else {
			Warning("Attack\n");
			// player released attack button, fire away!
			PrimaryAttack();
			gun_state = STATE_OFF;
			SetThink(NULL);
			pHL2MPPlayer->SetAmmoCount(0, m_iPrimaryAmmoType);
		}
			
		break;

	case STATE_DRAINING:
		if (owner->m_nButtons & IN_ATTACK) {
			if (charge < 100) {
				gun_state = STATE_CHARGING;
				SetThink(&CWeaponEngyDestroy::ChargeThink);
				SetNextThink(gpGlobals->curtime + 0.1f);
				Warning("Resuming Charge\n");
			} else {
				gun_state = STATE_CHARGED;
				SetThink(NULL);
				Warning("Holding Full Charge\n");
			}
		} else {
			if (charge <= 0) {
				gun_state = STATE_OFF;
				SetThink(NULL);

				Warning("Drained\n");
			}
		}
		break;	

	default:
		break;
	}
	
	if (!m_bInReload) {
		WeaponIdle();
	}
}

void CWeaponEngyDestroy::DamageOwner(void) {
#ifndef CLIENT_DLL
	CBasePlayer *bp;
	Vector dir;
	Vector start;
	Vector end;
	trace_t tr;

	if ((bp = ToBasePlayer(GetOwner())) == NULL)
		return;

	//
	// push player backwards, significantly!
	//
	dir = bp->GetAutoaimVector(0) * -1.0f;
	end = bp->Weapon_ShootPosition();
	start = start + (dir * ENGY_WELDER_RANGE * 4);

	UTIL_TraceLine(start, end, MASK_ALL, bp, COLLISION_GROUP_NONE, &tr);

	CTakeDamageInfo info(this, GetOwner(), DISCHARGE_OWNER_DMG, DMG_PLASMA);
	info.SetAmmoType(m_iPrimaryAmmoType);

	CalculateMeleeDamageForce(&info, dir, start, 3.0f);
	bp->DispatchTraceAttack(info, dir, &tr);
	ApplyMultiDamage();
#endif
}

void CWeaponEngyDestroy::ChargeThink(void) {
	int charge;
	CHL2MP_Player *p;
	
	SetNextThink(gpGlobals->curtime + 0.1f);

	p = ToHL2MPPlayer(GetOwner());
	if (!p)
		return;

	charge = p->GetAmmoCount(m_iPrimaryAmmoType);

	charge += 2;
	if (charge > 100)
		charge = 100;

	p->SetAmmoCount(charge, m_iPrimaryAmmoType);
}

void CWeaponEngyDestroy::DrainThink(void) {
	int charge;
	CHL2MP_Player *p;

	SetNextThink(gpGlobals->curtime + 0.1f);

	p = ToHL2MPPlayer(GetOwner());
	if (!p)
		return;

	charge = p->GetAmmoCount(m_iPrimaryAmmoType);
	
	charge -= 3;
	if (charge < 0)
		charge = 0;

	p->SetAmmoCount(charge, m_iPrimaryAmmoType);
}

//
// Secondary attack to destroy when held down
//
void CWeaponEngyDestroy::PrimaryAttack(void) {

#ifndef CLIENT_DLL
	Vector start;
	Vector end;
	Vector dir;
	trace_t tr;
	CBasePlayer *p;
	CBaseEntity *ent;
	CHumanMateriel *hmat;
	CSpiderMateriel *smat;
	CHL2MP_Player *pmat;

	p = ToBasePlayer(GetOwner());
	if (p) {
		dir = p->GetAutoaimVector(0);
		start = p->Weapon_ShootPosition();
		end = start + (dir * ENGY_WELDER_RANGE);

		UTIL_TraceLine(start, end, MASK_ALL, p, COLLISION_GROUP_NONE, &tr);

		ent = tr.m_pEnt;

		if (ent) {
			if ((hmat = dynamic_cast<CHumanMateriel *>(ent)) != NULL) {
				CTakeDamageInfo info(this, this, 9999, DMG_BULLET | DMG_ALWAYSGIB);
				info.SetAmmoType(m_iPrimaryAmmoType);

				CalculateMeleeDamageForce(&info, dir, start, 0.01f);
				hmat->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			} else if ((smat = dynamic_cast<CSpiderMateriel *>(ent)) != NULL) {
				CTakeDamageInfo info(this, this, DISCHARGE_ENT_DMG, DMG_PLASMA);
				info.SetAmmoType(m_iPrimaryAmmoType);

				CalculateMeleeDamageForce(&info, dir, start, 0.01f);
				smat->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			} else if ((pmat = ToHL2MPPlayer(ent)) != NULL) {
				CTakeDamageInfo info(this, GetOwner(), DISCHARGE_PLAYER_DMG, DMG_PLASMA);
				info.SetAmmoType(m_iPrimaryAmmoType);

				CalculateMeleeDamageForce(&info, dir, start, 4.0f);
				pmat->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			} else {
				//
				// infested corpses, server-side ragdolls and other non-materiel
				// ( must be specific to avoid NULL )
				//
				CTakeDamageInfo info(this, this, DISCHARGE_ENT_DMG, DMG_PLASMA);
				info.SetAmmoType(m_iPrimaryAmmoType);

				CalculateMeleeDamageForce(&info, dir, start, 0.01f);
				smat->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			}
		}
	}

#endif

	WeaponSound(SINGLE);
	m_flNextSecondaryAttack = gpGlobals->curtime + ENGY_REFIRE;

}