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

//-----------------------------------------------------------------------------
// CWeaponEngyDestroy
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponEngyDestroy, DT_WeaponEngyDestroy )

BEGIN_NETWORK_TABLE( CWeaponEngyDestroy, DT_WeaponEngyDestroy )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponEngyDestroy )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_engy_destroy, CWeaponEngyDestroy );
PRECACHE_WEAPON_REGISTER( weapon_engy_destroy );

#define	MEGACANNON_MODEL "models/weapons/v_superphyscannon.mdl"
#define	MEGACANNON_SKIN	1

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

	BaseClass::ItemPostFrame();
}

//
// Secondary attack to destroy when held down
//
void CWeaponEngyDestroy::SecondaryAttack(void) {

#ifndef CLIENT_DLL
	Vector start;
	Vector end;
	Vector dir;
	trace_t tr;
	CBasePlayer *p;
	CBaseEntity *ent;
	CHumanMateriel *hmat;

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
				CalculateMeleeDamageForce(&info, dir, start, 0.01f);
				hmat->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			}
		}
	}

#endif

	WeaponSound(SINGLE);
	m_flNextSecondaryAttack = gpGlobals->curtime + ENGY_REFIRE;

}

void CWeaponEngyDestroy::PrimaryAttack(void) {
	//BaseClass::PrimaryAttack();
}

#ifndef CLIENT_DLL
void CWeaponEngyDestroy::DisplayUsageHudHint(void) {
	UTIL_HudHintText(GetOwner(), "%+attack2%  destroy structure");
	BaseClass::DisplayUsageHudHint();
}
#endif
