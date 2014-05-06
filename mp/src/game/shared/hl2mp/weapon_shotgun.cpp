//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_shotgun.h"

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponShotgun, DT_WeaponShotgun )

BEGIN_NETWORK_TABLE( CWeaponShotgun, DT_WeaponShotgun )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bNeedPump ) ),
	RecvPropBool( RECVINFO( m_bDelayedFire1 ) ),
	RecvPropBool( RECVINFO( m_bDelayedFire2 ) ),
	RecvPropBool( RECVINFO( m_bDelayedReload ) ),
	RecvPropInt( RECVINFO( currentAmmoType ) ),
#else
	SendPropBool( SENDINFO( m_bNeedPump ) ),
	SendPropBool( SENDINFO( m_bDelayedFire1 ) ),
	SendPropBool( SENDINFO( m_bDelayedFire2 ) ),
	SendPropBool( SENDINFO( m_bDelayedReload ) ),
	SendPropInt( SENDINFO( currentAmmoType ), 6 ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponShotgun )
	DEFINE_PRED_FIELD( m_bNeedPump, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDelayedFire1, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDelayedFire2, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDelayedReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( currentAmmoType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_shotgun, CWeaponShotgun );
PRECACHE_WEAPON_REGISTER(weapon_shotgun);

acttable_t	CWeaponShotgun::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_SHOTGUN,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_SHOTGUN,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_SHOTGUN,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_SHOTGUN,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_SHOTGUN,					false },
};

IMPLEMENT_ACTTABLE(CWeaponShotgun);

#ifndef CLIENT_DLL

void CWeaponShotgun::Spawn(void) {
	BaseClass::Spawn();
	currentAmmoType = m_iPrimaryAmmoType;
}

#endif

void CWeaponShotgun::ToggleAmmoType(void) {
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;

	m_iClip1 = 0;
	if (currentAmmoType == m_iPrimaryAmmoType)
		currentAmmoType = m_iSecondaryAmmoType;
	else
		currentAmmoType = m_iPrimaryAmmoType;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponShotgun::StartReload( void )
{
	if ( m_bNeedPump )
		return false;

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if (pOwner->GetAmmoCount(currentAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;


	int j = MIN(1, pOwner->GetAmmoCount(currentAmmoType));

	if (j <= 0)
		return false;

	SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );

	// Make shotgun shell visible
	SetBodygroup(1,0);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	m_bInReload = true;
	return true;
}

void CWeaponShotgun::Drop(const Vector &vecVelocity) {
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponShotgun::Reload( void )
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if (pOwner->GetAmmoCount(currentAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	int j = MIN(1, pOwner->GetAmmoCount(currentAmmoType));

	if (j <= 0)
		return false;

	FillClip();
	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);
	SendWeaponAnim( ACT_VM_RELOAD );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
//-----------------------------------------------------------------------------
void CWeaponShotgun::FinishReload( void )
{
	// Make shotgun shell invisible
	SetBodygroup(1,1);

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
//-----------------------------------------------------------------------------
void CWeaponShotgun::FillClip( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	// Add them to the clip
	if ( pOwner->GetAmmoCount( currentAmmoType ) > 0 ) {
		if ( Clip1() < GetMaxClip1() ) {
			m_iClip1++;
			pOwner->RemoveAmmo( 1, currentAmmoType );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play weapon pump anim
//-----------------------------------------------------------------------------
void CWeaponShotgun::Pump( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;
	
	m_bNeedPump = false;

	if ( m_bDelayedReload ) {
		m_bDelayedReload = false;
		StartReload();
	}
	
	WeaponSound( SPECIAL1 );

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_PUMP );

	pOwner->m_flNextAttack	= gpGlobals->curtime + SequenceDuration();
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
}

void CWeaponShotgun::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponShotgun::PrimaryAttack( void ) {
	CBasePlayer *pPlayer;
	trace_t tr;
	Vector vecSrc;
	Vector vecAiming;
	Vector fwd;
	Vector endPos;

	pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_iClip1 -= 1;

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );


	vecSrc		= pPlayer->Weapon_ShootPosition( );
	vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );	

	if (currentAmmoType == m_iPrimaryAmmoType) {
		FireBulletsInfo_t info( 7, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
		info.m_pAttacker = pPlayer;

		// Fire the bullets, and force the first shot to be perfectly accuracy
		pPlayer->FireBullets( info );

	} else {

		// attempt to trace a line really far
		pPlayer->EyeVectors(&fwd);

		if (!(pPlayer->GetFlags() & FL_DUCKING)) {
			fwd.x += RandomFloat( -0.4f, 0.4f );
			fwd.y += RandomFloat( -0.4f, 0.4f );
			fwd.z += RandomFloat( -0.4f, 0.4f );
		}

		endPos = vecSrc + (fwd * MAX_TRACE_LENGTH);
		vecSrc = vecSrc + (fwd * 10);
		UTIL_TraceLine(vecSrc, endPos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

		// back off a bit, spawn explosion
		//endPos = tr.endpos - (fwd);

#ifndef CLIENT_DLL
		// 2nd to last arg = damage force...?
		//ExplosionCreate(tr.endpos, vec3_angle, pPlayer, XP_SHELL_DMG, XP_SHELL_RAD, SF_ENVEXPLOSION_NODLIGHTS, 100.0f, pPlayer);
		
		//ExplosionCreate(tr.endpos, GetAbsAngles(), pPlayer, 400, 100, 0, 100.0f);

		if (tr.DidHit()) {
			endPos = tr.endpos + (tr.plane.normal * 10.0f);
		} else {
			endPos = tr.endpos;
		}

		//
		// this is needed, or the firing player won't see/hear the explosion
		//
		CDisablePredictionFiltering  disabler;
		int exflags = SF_ENVEXPLOSION_NOFIREBALL;
			
		ExplosionCreate( endPos, GetAbsAngles(), pPlayer, XP_SHELL_DMG, XP_SHELL_RAD, exflags, 0.0f, this);

#endif

	}
	
	QAngle punch;
	punch.Init( SharedRandomFloat( "shotgunpax", -2, -1 ), SharedRandomFloat( "shotgunpay", -2, 2 ), 0 );
	pPlayer->ViewPunch( punch );

	if (!m_iClip1 && pPlayer->GetAmmoCount(currentAmmoType) <= 0) {
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	m_bNeedPump = true;
}

void CWeaponShotgun::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
		return;

	if ( m_bNeedPump && ( pOwner->m_nButtons & IN_RELOAD ) ) {
		m_bDelayedReload = true;
	}


	if (m_bInReload) {
		//
		// once in reload, don't let the player hit IN_ATTACK2 to toggle
		// an ammo switch, that would be wasteful
		//
		if ((pOwner->m_nButtons & IN_ATTACK ) && (m_iClip1 >= 1) && !m_bNeedPump ) {

			// If I'm primary firing and have one round stop reloading and fire
			m_bInReload		= false;
			m_bDelayedFire1 = true;

		} else if (m_flNextPrimaryAttack <= gpGlobals->curtime) {

			// If out of ammo end reloading
			if (pOwner->GetAmmoCount(currentAmmoType) <= 0) {
				FinishReload();
				return;
			}
			
			if (m_iClip1 < GetMaxClip1()) {
				// If clip not full reload again
				Reload();
				return;

			} else {
				// Clip full, stop reloading
				FinishReload();
				return;

			}

		}
	} else {
		// not in the middle of reloading, so...
		// Make shotgun shell invisible
		SetBodygroup(1,1);
	}

	if ((m_bNeedPump) && (m_flNextPrimaryAttack <= gpGlobals->curtime)) {
		//
		// shotgun needed a pump, wasn't in the middle of a reload
		// the shotgun tries to stay pumped/ready
		//
		Pump();
		return;
	}

	//
	// if we get there, we might have a delayedFire1 from above
	// but if we were in the middle of reloading, those actions will
	// have already been handled
	//

	if ( (m_bDelayedFire1 || pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime) {
		
		m_bDelayedFire1 = false;

		if (m_iClip1 <= 0) {

			//
			// player indicated attack, but there was no ammo in the clip
			// don't ToggleAmmoType() because the player could accidentally
			// load explosive shells (that could damage teammates!)
			//
			if (!pOwner->GetAmmoCount(currentAmmoType)) {
				DryFire();
			} else {
				StartReload();
			}

		} else {
			//
			// player indicated attack, and clip had ammo
			// but only fire if the player pressed attack this frame, not if
			// this was a delayedFire1 (wait until next frame)
			//
			if (pOwner->m_afButtonPressed & IN_ATTACK ) {
				 m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			PrimaryAttack();
		}
	}


	if ( pOwner->m_nButtons & IN_RELOAD && !m_bInReload )  {
		//
		// player wanted to reload, and we weren't already doing it,
		// so start reloading!
		//
		StartReload();

	} else if (pOwner->m_nButtons & IN_ATTACK2 && m_flNextSecondaryAttack < gpGlobals->curtime) {

		//
		// finally, if there's nothing else going on, and the player
		// wants to toggle ammo, do it, and start a reload sequence
		//
		ToggleAmmoType();
		if (pOwner->GetAmmoCount(currentAmmoType) > 0) {
			StartReload();
		} else {
			//
			// swap original ammo type back in-case player
			// goes to depot for more ammo
			//
			ToggleAmmoType();
			WeaponIdle();
		}
		
	} else {
		
		WeaponIdle( );
		return;
	}

}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponShotgun::CWeaponShotgun( void )
{
	m_bReloadsSingly = true;

	m_bNeedPump		= false;
	m_bDelayedFire1 = false;
	m_bDelayedFire2 = false;

	m_fMinRange1		= 0.0;
	m_fMaxRange1		= 500;
	m_fMinRange2		= 0.0;
	m_fMaxRange2		= 200;

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShotgun::ItemHolsterFrame( void )
{
	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;

	// If it's been longer than three seconds, reload
	if ( ( gpGlobals->curtime - m_flHolsterTime ) > sk_auto_reload_time.GetFloat() )
	{
		// Reset the timer
		m_flHolsterTime = gpGlobals->curtime;
	
		if ( GetOwner() == NULL )
			return;

		if ( m_iClip1 == GetMaxClip1() )
			return;

		// Just load the clip with no animations
		int ammoFill = MIN( (GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount( currentAmmoType ) );
		
		GetOwner()->RemoveAmmo( ammoFill, currentAmmoType );
		m_iClip1 += ammoFill;
	}
}
