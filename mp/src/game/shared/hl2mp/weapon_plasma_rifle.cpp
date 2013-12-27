//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		PlasmaRifle - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp/weapon_plasma_rifle.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "beam_shared.h"
#include "takedamageinfo.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "ai_basenpc.h"
	#include "explode.h"
	#include "weapon_gauss.h"
#endif

#include "tier0/memdbgon.h"

#define RAILGUN_BEAM_SPRITE "sprites/laserbeam.vmt"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPlasmaRifle, DT_WeaponPlasmaRifle )

BEGIN_NETWORK_TABLE( CWeaponPlasmaRifle, DT_WeaponPlasmaRifle )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponPlasmaRifle )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_plasma_rifle, CWeaponPlasmaRifle );
PRECACHE_WEAPON_REGISTER( weapon_plasma_rifle );

acttable_t	CWeaponPlasmaRifle::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_AR2,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_AR2,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_AR2,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_AR2,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_AR2,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_AR2,					false },
};

IMPLEMENT_ACTTABLE(CWeaponPlasmaRifle);

#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponPlasmaRifle )
END_DATADESC()
#endif


CWeaponPlasmaRifle::CWeaponPlasmaRifle( void ) {
	invisible = true;
}

void CWeaponPlasmaRifle::Drop( const Vector &vecVelocity ) {
}


void CWeaponPlasmaRifle::ItemPostFrame( void ) {
	CBasePlayer *owner;
	CHL2MP_Player *p;

	owner = ToBasePlayer(GetOwner());
	if (!owner)
		return;

	p = ToHL2MPPlayer(owner);
	if (!p)
		return;

	if (owner->m_nButtons & IN_ATTACK) {
		m_nShotsFired = 0;
		if (m_flNextPrimaryAttack < gpGlobals->curtime && BaseClass::HasAnyAmmo()) {
			if (p->PlasmaReady()) {
				FireRifle();
				return;
			}
		}
	}
	if (owner->m_nButtons & IN_ATTACK2) {
		if (m_flNextSecondaryAttack < gpGlobals->curtime && owner->GetAmmoCount(m_iPrimaryAmmoType) > 75) {
			if (p->PlasmaReady()) {
				m_flNextSecondaryAttack = gpGlobals->curtime + RAILGUN_REFIRE;
				FireRailgun();
				return;
			}
		}
	}

	WeaponIdle();
}

void CWeaponPlasmaRifle::FireRifle(void) {

	// do necessary callbacks to player for plasma use
	// then fire some plasma in a way similar to SMG/AR2
	int iBulletsToFire;
	int maxBurstAmmo;
	CBasePlayer *pPlayer;
	CHL2MP_Player *pHL2MPPlayer;
	
	pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	pHL2MPPlayer = ToHL2MPPlayer( pPlayer );
	if (!pHL2MPPlayer)
		return;
	
	m_nShotsFired++;
	maxBurstAmmo = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	iBulletsToFire = 0;

	while (m_flNextPrimaryAttack <= gpGlobals->curtime && iBulletsToFire < GetMaxBurst() && maxBurstAmmo > 0) {
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack += PLASMA_RIFLE_REFIRE;
		iBulletsToFire++;
		maxBurstAmmo--;

		#ifndef CLIENT_DLL
		pHL2MPPlayer->PlasmaShot();
		#endif
	}

	if (m_flNextPrimaryAttack < gpGlobals->curtime)
		m_flNextPrimaryAttack = gpGlobals->curtime + PLASMA_RIFLE_REFIRE;

	
		// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pHL2MPPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	info.m_vecSpread = pHL2MPPlayer->GetAttackSpread(this);
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	FireBullets( info );

	//Factor in the view kick
	AddViewKick();
	
	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0) {
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	
}

void CWeaponPlasmaRifle::FireRailgun(void) {
	CBasePlayer *pPlayer;
	CHL2MP_Player *pHL2MPPlayer;
	CBaseEntity *hit;
	float width;
	Vector startPos;
	Vector endPos;
	Vector fwd;
	
	pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	pHL2MPPlayer = ToHL2MPPlayer( pPlayer );
	if (!pHL2MPPlayer)
		return;

	WeaponSound(SPECIAL1);

	pPlayer->DoMuzzleFlash();

#ifndef CLIENT_DLL
	pHL2MPPlayer->RailgunShot();
#endif

	AddViewKick();

	SendWeaponAnim( GetPrimaryAttackActivity() );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	//ToHL2MPPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );


	//
	// this is based on jeep code...
	//
	trace_t tr;
	startPos = pPlayer->Weapon_ShootPosition();
	pPlayer->EyeVectors(&fwd);
	endPos = startPos + (fwd * MAX_TRACE_LENGTH);

	UTIL_TraceLine(startPos, endPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	ClearMultiDamage();
	
	hit = tr.m_pEnt;
	if (!(tr.DidHitWorld() && !(tr.surface.flags & SURF_SKY))) {
		CTakeDamageInfo info(this, pPlayer, RAILGUN_DMG, DMG_PLASMA);
		CalculateBulletDamageForce(&info, m_iPrimaryAmmoType, fwd, tr.endpos, 4.0f);
		hit->DispatchTraceAttack(info, fwd, &tr);
	}

	ApplyMultiDamage();

	// impact effect?

	width = RAILGUN_BEAM_WIDTH;

#ifndef CLIENT_DLL
	//Tracer down the middle
    UTIL_Tracer( startPos, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );

	//Draw the main beam shaft
	CBeam *pBeam = CBeam::BeamCreate( RAILGUN_BEAM_SPRITE, 0.5 );

	pBeam->SetStartPos( startPos );
	pBeam->PointEntInit( tr.endpos, this );
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );
	pBeam->SetWidth( width );
	pBeam->SetEndWidth( 0.05f );
	pBeam->SetBrightness( 255 );
	pBeam->SetColor( 255, 185+random->RandomInt( -16, 16 ), 40 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );

	//Draw electric bolts along shaft
	pBeam = CBeam::BeamCreate( RAILGUN_BEAM_SPRITE, 3.0f );

	pBeam->SetStartPos( startPos );
	pBeam->PointEntInit( tr.endpos, this );
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );

	pBeam->SetBrightness( random->RandomInt( 64, 255 ) );
	pBeam->SetColor( 255, 255, 150+random->RandomInt( 0, 64 ) );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );
	pBeam->SetNoise( 1.6f );
	pBeam->SetEndWidth( 0.1f );
#endif

}

void CWeaponPlasmaRifle::Precache(void) {
	PrecacheModel(RAILGUN_BEAM_SPRITE);
	BaseClass::Precache();
}

#ifndef CLIENT_DLL
void CWeaponPlasmaRifle::DisplayUsageHudHint(void) {
	UTIL_HudHintText(GetOwner(), "%+attack%  fire plasma rifle\n%+attack2%  discharge all plasma\n%+jump%  hold for jetpack");
	BaseClass::DisplayUsageHudHint();
}
#endif
