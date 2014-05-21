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
	#include "fx_interpvalue.h"
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

#define	SPRITE_SCALE	128.0f

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	ENGY_RANGE	85.0f
#define	ENGY_REFIRE	5.0f
#define ENGY_WELDER_RANGE 115.0f

#define ITEM_UPDATE_INTERVAL	0.5f

#define DESTROY_CHARGE_TIME 7.0f
#define CHARGE_WARN_TIME 2.0f
#define DEFAULT_WARN_BEEPS 4

#define DISCHARGE_OWNER_DMG 35
#define DISCHARGE_PLAYER_DMG 55
#define DISCHARGE_ENT_DMG 25
#define DEFAULT_PLAYER_PUSH_SPEED 600

#define CHARGE_BEEP_DELAY 1.0f
#define CHARGE_BEEP_SND "Buttons.snd16"

#ifdef CLIENT_DLL

	//Precahce the effects
	CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectDestroyGun )
	CLIENTEFFECT_MATERIAL( "sprites/orangelight1" )
	CLIENTEFFECT_MATERIAL( "sprites/orangelight1_noz" )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_GLOW_SPRITE )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_ENDCAP_SPRITE )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_CENTER_GLOW )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_BLAST_SPRITE )
	CLIENTEFFECT_REGISTER_END()

#endif


//-----------------------------------------------------------------------------
// CWeaponEngyDestroy
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponEngyDestroy, DT_WeaponEngyDestroy )

BEGIN_NETWORK_TABLE( CWeaponEngyDestroy, DT_WeaponEngyDestroy )
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(warnBeepsLeft)),
	RecvPropFloat(RECVINFO(nextWarnBeep)),
	RecvPropBool(RECVINFO(clawsOpen)),
	RecvPropInt(RECVINFO(m_EffectState)),
#else
	SendPropInt(SENDINFO(warnBeepsLeft), 3, SPROP_UNSIGNED),
	SendPropFloat(SENDINFO(nextWarnBeep)),
	SendPropBool(SENDINFO(clawsOpen)),
	SendPropInt(SENDINFO(m_EffectState)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponEngyDestroy)
	DEFINE_PRED_FIELD(m_EffectState, FIELD_INTEGER,	FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(clawsOpen, FIELD_BOOLEAN,	FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

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


enum
{
	EFFECT_NONE,
	EFFECT_CLOSED,
	EFFECT_READY,
	EFFECT_HOLDING,
	EFFECT_LAUNCH,
};


CWeaponEngyDestroy::CWeaponEngyDestroy( void ) {
	SetWeaponVisible(false);
	AddEffects(EF_NODRAW);
	m_flNextItemStatus = 0.0f;
	clawsOpen = false;
	m_EffectState = (int)EFFECT_NONE;
	gun_state = STATE_OFF;

#ifdef CLIENT_DLL
	m_nOldEffectState = EFFECT_NONE;
	oldClawsOpen = false;
#endif
}

void CWeaponEngyDestroy::Precache(void) {
	PrecacheModel(PHYSCANNON_BEAM_SPRITE);
	PrecacheModel(PHYSCANNON_BEAM_SPRITE_NOZ);

	PrecacheScriptSound( "Weapon_PhysCannon.HoldSound" );
	PrecacheScriptSound( CHARGE_BEEP_SND );
	BaseClass::Precache();
}

void CWeaponEngyDestroy::UpdateOnRemove(void) {
	DestroyEffects();
	BaseClass::UpdateOnRemove();
}

float CWeaponEngyDestroy::GetDamageForActivity( Activity hitActivity ) {
	return 0.0f;
}

bool CWeaponEngyDestroy::Deploy(void) {
	bool ret;
	CHL2MP_Player *p;

	CloseElements();
	StopEffects();
	gun_state = STATE_OFF;
	clawsOpen = false;
	m_EffectState = (int)EFFECT_NONE;

	p = ToHL2MPPlayer(GetOwner());
	if (p) {
		p->SetAmmoCount(0, m_iPrimaryAmmoType);
	}
	
	m_flNextItemStatus = 0.0f;

	ret = BaseClass::Deploy();
	AddEffects(EF_NODRAW);

#ifdef CLIENT_DLL
	m_nOldEffectState = EFFECT_NONE;
	oldClawsOpen = false;
#endif

	StopEffects();

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
void CWeaponEngyDestroy::ItemStatusUpdate(CBasePlayer *player, int health, int armor, int maxhealth) {
	CSingleUserRecipientFilter user(player);
	UserMessageBegin(user, "ItemInfo");
		WRITE_SHORT(armor);
		WRITE_SHORT(health);
		WRITE_SHORT(maxhealth);
	MessageEnd();
}

void CWeaponEngyDestroy::Spawn(void) {
	BaseClass::Spawn();
	SetAmmoType(GetAmmoDef()->Index("engy_destroy"));

	gun_state = STATE_OFF;
	clawsOpen = false;
	m_EffectState = (int)EFFECT_NONE;

	StopEffects();
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
	int maxHealth;
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
			maxHealth = -1;

			if (ent && ent->GetTeamNumber() == TEAM_HUMANS) {
				if ((other = dynamic_cast<CHL2MP_Player *>(ent)) != NULL) {
					health = other->GetHealth();
					maxHealth = other->GetMaxHealth();
					armor = other->ArmorValue();
				} else if ((tele = dynamic_cast<CTeleporterEntity*>(ent)) != NULL) {
					health = tele->GetHealth();
					maxHealth = tele->GetMaxHealth();
				} else if ((ammo = dynamic_cast<CAmmoCrate *>(ent)) != NULL) {
					health = ammo->GetHealth();
					maxHealth = ammo->GetMaxHealth();
				} else if ((medipad = dynamic_cast<CMedipadEntity *>(ent)) != NULL) {
					health = medipad->GetHealth();
					maxHealth = medipad->GetMaxHealth();
				} else if ((mgturret = dynamic_cast<CMGTurretEntity *>(ent)) != NULL) {
					health = mgturret->GetHealth();
					maxHealth = mgturret->GetMaxHealth();
				} else if ((det = dynamic_cast<CDetectorEntity *>(ent)) != NULL) {
					health = det->GetHealth();
					maxHealth = det->GetMaxHealth();
				} else if ((missile_turret = dynamic_cast<CMSLTurretEntity *>(ent)) != NULL) {
					health = missile_turret->GetHealth();
					maxHealth = missile_turret->GetMaxHealth();
				}
			}

			ItemStatusUpdate(p, health, armor, maxHealth);
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
			
			// start charging
			CloseElements();
			StopEffects();
			DoEffect(EFFECT_READY);

			if (GetMotorSound()) {
				(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.0f, 1.0f );
				(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 50, 1.0f );
			}

			WeaponSound(RELOAD);
			nextWarnBeep = gpGlobals->curtime + CHARGE_BEEP_DELAY;

		} else {
			StopEffects();
		}

		break;

	case STATE_CHARGING:
		if (owner->m_nButtons & IN_ATTACK) {
			if (charge >= 100) {
				gun_state = STATE_CHARGED;
				nextWarnBeep = gpGlobals->curtime + CHARGE_WARN_TIME;
				warnBeepsLeft = DEFAULT_WARN_BEEPS;
				SetThink(NULL);

				// full charge
				OpenElements();
				DoEffect(EFFECT_HOLDING);

				if (GetMotorSound()) {
					(CSoundEnvelopeController::GetController()).Play( GetMotorSound(), 0.0f, 50 );
					(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 100, 0.5f );
					(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.8f, 0.5f );
				}

				WeaponSound(SPECIAL2);
			} else {
				// do charging beeps
				if (nextWarnBeep < gpGlobals->curtime) {
					nextWarnBeep = gpGlobals->curtime + CHARGE_BEEP_DELAY - ChargeBeepQuickening(charge);
					ChargingBeep(charge);
				}
			}
		} else {
			// start draining
			gun_state = STATE_DRAINING;
			SetThink(&CWeaponEngyDestroy::DrainThink);
			SetNextThink(gpGlobals->curtime + 0.1f);
			
			CloseElements();
			StopEffects();

			WeaponSound(MELEE_HIT);
		}
		break;

	case STATE_CHARGED:
		if (owner->m_nButtons & IN_ATTACK) {
			if (gpGlobals->curtime > nextWarnBeep) {
				if (warnBeepsLeft > 0) {
					nextWarnBeep = gpGlobals->curtime + CHARGE_WARN_TIME;
					warnBeepsLeft--;

					if (warnBeepsLeft > 0) {
						WeaponSound(SPECIAL1);
					} else {
						// final warning beep
						WeaponSound(EMPTY);
					}
				} else {
					gun_state = STATE_OFF;
					SetThink(NULL);

					// damage to owner
					DamageOwner();
					CloseElements();
					StopEffects();
					SendWeaponAnim(ACT_VM_PRIMARYATTACK);

					WeaponSound(SPECIAL3);
				}
			}
		} else {
			// regular attack at full charge
			gun_state = STATE_OFF;
			SetThink(NULL);
			pHL2MPPlayer->SetAmmoCount(0, m_iPrimaryAmmoType);
			CloseElements();
			StopEffects();

			SendWeaponAnim(ACT_VM_PRIMARYATTACK);
			PrimaryAttack();

			WeaponSound(SINGLE);
		}
			
		break;

	case STATE_DRAINING:
		if (owner->m_nButtons & IN_ATTACK) {
			if (charge < 100) {
				// resume charging
				gun_state = STATE_CHARGING;
				SetThink(&CWeaponEngyDestroy::ChargeThink);
				SetNextThink(gpGlobals->curtime + 0.1f);
				CloseElements();
				StopEffects();
				DoEffect(EFFECT_READY);

				if (GetMotorSound()) {
					(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.0f, 1.0f );
					(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 50, 1.0f );
				}

				WeaponSound(RELOAD);
				nextWarnBeep = gpGlobals->curtime + CHARGE_BEEP_DELAY - ChargeBeepQuickening(charge);

			} else {
				// resume at full charge (warnings stay the same)
				gun_state = STATE_CHARGED;
				SetThink(NULL);
				
				OpenElements();
				DoEffect(EFFECT_HOLDING);

				if (GetMotorSound()) {
					(CSoundEnvelopeController::GetController()).Play( GetMotorSound(), 0.0f, 50 );
					(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 100, 0.5f );
					(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.8f, 0.5f );
				}

				WeaponSound(SPECIAL2);
			}
		} else {
			if (charge <= 0) {
				// completely drained
				gun_state = STATE_OFF;
				SetThink(NULL);

				CloseElements();
				StopEffects();

				WeaponSound(MELEE_MISS);
			}
		}
		break;	
	}
	
	if (!m_bInReload) {
		WeaponIdle();
	}
}

//
// return value less than CHARGE_BEEP_DELAY
// where it's higher as percent approaches 100
//

float CWeaponEngyDestroy::ChargeBeepQuickening(int percent) {
	return (CHARGE_BEEP_DELAY * ((float)percent / 100.0f));
}

//
// play the charge beep sound with varying pitch
//
void CWeaponEngyDestroy::ChargingBeep(int percent) {
#ifndef CLIENT_DLL
	CSoundParameters params;
	EmitSound_t ep;
	Vector vecOrigin;
	int pitch;

	pitch = percent + 70;

	if (GetParametersForSound( CHARGE_BEEP_SND, params, NULL ) == false)
			return;

	vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS(vecOrigin);
		
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
#endif
}

bool CWeaponEngyDestroy::Holster(CBaseCombatWeapon *pSwitchingTo) {
	CloseElements();
	StopEffects();
	DestroyEffects();
	StopLoopingSounds();

	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponEngyDestroy::DryFire(void) {
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	WeaponSound(EMPTY);
}

void CWeaponEngyDestroy::PrimaryFireEffect(void) {
	CBasePlayer *pOwner;
	
	pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->ViewPunch( QAngle(-6, SharedRandomInt( "physcannonfire", -2,2) ,0) );
	
#ifndef CLIENT_DLL
	color32 white = { 245, 245, 255, 32 };
	UTIL_ScreenFade( pOwner, white, 0.1f, 0.0f, FFADE_IN );
#endif
}

void CWeaponEngyDestroy::ShockEntityEffect(CBaseEntity *pEntity, const Vector &forward, trace_t &tr) {
	#ifndef CLIENT_DLL
	DoEffect(EFFECT_LAUNCH, &tr.endpos);
	#endif
	
	PrimaryFireEffect();
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
}

float CWeaponEngyDestroy::TraceLength() {
	return 250.0f;
}

CSoundPatch *CWeaponEngyDestroy::GetMotorSound(void) {
	if (m_sndMotor == NULL) {
		CPASAttenuationFilter filter(this);
		
		m_sndMotor = (CSoundEnvelopeController::GetController()).SoundCreate
		(
			filter,
			entindex(),
			CHAN_STATIC,
			"Weapon_PhysCannon.HoldSound",
			ATTN_NORM
		);
	}

	return m_sndMotor;
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

	bp->ApplyAbsVelocityImpulse(DEFAULT_PLAYER_PUSH_SPEED * dir);
	bp->SetGroundEntity(NULL);

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
	float pushSpeed;

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
				ent->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			} else if ((smat = dynamic_cast<CSpiderMateriel *>(ent)) != NULL) {
				CTakeDamageInfo info(this, this, DISCHARGE_ENT_DMG, DMG_PLASMA);
				info.SetAmmoType(m_iPrimaryAmmoType);
				CalculateMeleeDamageForce(&info, dir, start, 1.0f);
				ent->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			} else if ((pmat = ToHL2MPPlayer(ent)) != NULL) {
				CTakeDamageInfo info(this, GetOwner(), DISCHARGE_PLAYER_DMG, DMG_PLASMA);
				info.SetAmmoType(m_iPrimaryAmmoType);

				//
				// ghetto way of adjusting player push speed
				// according to player size/class... it would be
				// better to use player mass
				//
				switch (pmat->m_iClassNumber) {
				case CLASS_MECH_IDX:
				case CLASS_STALKER_IDX:
					pushSpeed = 0.3f * DEFAULT_PLAYER_PUSH_SPEED;
					break;
				
				case CLASS_EXTERMINATOR_IDX:
				case CLASS_GUARDIAN_IDX:
					pushSpeed = 0.5f * DEFAULT_PLAYER_PUSH_SPEED;
					break;
				
				case CLASS_STINGER_IDX:
					pushSpeed = 0.8f * DEFAULT_PLAYER_PUSH_SPEED;
					break;

				case CLASS_HATCHY_IDX:
				case CLASS_KAMI_IDX:
					pushSpeed = 2.0f * DEFAULT_PLAYER_PUSH_SPEED;
					break;

				default:
					pushSpeed = 1.0f * DEFAULT_PLAYER_PUSH_SPEED;
					break;
				}

				pmat->ApplyAbsVelocityImpulse(pushSpeed * dir);
				pmat->SetGroundEntity(NULL);

				CalculateMeleeDamageForce(&info, dir, start, 1.0f);
				ent->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			} else {
				//
				// infested corpses, server-side ragdolls and other non-materiel
				//
				CTakeDamageInfo info(this, this, DISCHARGE_ENT_DMG, DMG_PLASMA);
				info.SetAmmoType(m_iPrimaryAmmoType);

				CalculateMeleeDamageForce(&info, dir, start, 0.0f);
				ent->DispatchTraceAttack(info, dir, &tr);
				ApplyMultiDamage();
			}

			ShockEntityEffect(ent, dir, tr);

		}
	}

#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + ENGY_REFIRE;

	if (GetMotorSound()) {
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.0f, 1.0f );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 50, 1.0f );
	}

}

void CWeaponEngyDestroy::OpenElements(void) {
	CBasePlayer *pOwner;

	if (clawsOpen)
		return;

	pOwner = ToBasePlayer( GetOwner() );
	if (pOwner == NULL)
		return;

	SendWeaponAnim(ACT_VM_IDLE);

	clawsOpen = true;

	DoEffect(EFFECT_READY);

#ifdef CLIENT

	// Element prediction 
	m_ElementParameter.InitFromCurrent( 1.0f, 0.2f, INTERP_SPLINE );
	oldClawsOpen = true;

#endif
}

void CWeaponEngyDestroy::StopLoopingSounds(void) {
	if (m_sndMotor != NULL) {
		 (CSoundEnvelopeController::GetController()).SoundDestroy( m_sndMotor );
		 m_sndMotor = NULL;
	}
#ifndef CLIENT_DLL
	BaseClass::StopLoopingSounds();
#endif
}

void CWeaponEngyDestroy::CloseElements(void) {
	CBasePlayer *pOwner;

	if ( clawsOpen == false )
		return;

	pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;

	SendWeaponAnim( ACT_VM_IDLE );

	clawsOpen = false;

	if ( GetMotorSound() )
	{
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.0f, 1.0f );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 50, 1.0f );
	}
	
	DoEffect(EFFECT_CLOSED);

#ifdef CLIENT
	// Element prediction 
	m_ElementParameter.InitFromCurrent( 0.0f, 0.5f, INTERP_SPLINE );
	oldClawsOpen = false;
#endif
}

inline float CWeaponEngyDestroy::SpriteScaleFactor() {
	return 1.0f;
}

#ifdef CLIENT_DLL
void CWeaponEngyDestroy::OnDataChanged(DataUpdateType_t type) {
	BaseClass::OnDataChanged( type );

	if (type == DATA_UPDATE_CREATED) {
		SetNextClientThink(CLIENT_THINK_ALWAYS);

		C_BaseAnimating::AutoAllowBoneAccess boneaccess(true, false);
		StartEffects();
		StopEffects();
	}

	if (m_nOldEffectState != m_EffectState) {
		DoEffect(m_EffectState);
		m_nOldEffectState = m_EffectState;
	}

	if (oldClawsOpen != clawsOpen) {
		if (clawsOpen) {
			m_ElementParameter.InitFromCurrent( 1.0f, 0.2f, INTERP_SPLINE );
		} else {	
			m_ElementParameter.InitFromCurrent( 0.0f, 0.5f, INTERP_SPLINE );
		}

		oldClawsOpen = clawsOpen;
	}
}

void CWeaponEngyDestroy::UpdateElementPosition(void) {
	CBasePlayer *pOwner;
	CBaseViewModel *vm;
	float flElementPosition;

	pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	flElementPosition = m_ElementParameter.Interp(gpGlobals->curtime);

	if (ShouldDrawUsingViewModel()) {
		if (pOwner) {
			vm = pOwner->GetViewModel();
			if (vm) {
				vm->SetPoseParameter("active", flElementPosition);
			}
		}
	} else {
		SetPoseParameter("active", flElementPosition);
	}
}

void CWeaponEngyDestroy::ClientThink(void) {
	UpdateElementPosition();
	DoEffectIdle();
}

#endif

void CWeaponEngyDestroy::DoEffectIdle( void ) {
#ifdef CLIENT_DLL

	StartEffects();

	//if ( ShouldDrawUsingViewModel() )
	{
		// Turn on the glow sprites
		for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ ) {
			m_Parameters[i].GetScale().SetAbsolute( random->RandomFloat( 0.075f, 0.05f ) * SPRITE_SCALE );
			m_Parameters[i].GetAlpha().SetAbsolute( random->RandomInt( 24, 32 ) );
		}

		// Turn on the glow sprites
		for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ ) {
			m_Parameters[i].GetScale().SetAbsolute( random->RandomFloat( 3, 5 ) );
			m_Parameters[i].GetAlpha().SetAbsolute( random->RandomInt( 200, 255 ) );
		}

		if ( m_EffectState != EFFECT_HOLDING ) {
			// Turn beams off
			m_Beams[0].SetVisible( false );
			m_Beams[1].SetVisible( false );
			m_Beams[2].SetVisible( false );
		}
	}
#endif
}

void CWeaponEngyDestroy::DestroyEffects(void) {
#ifdef CLIENT_DLL
	// Free our beams
	m_Beams[0].Release();
	m_Beams[1].Release();
	m_Beams[2].Release();

#endif

	// Stop everything
	StopEffects();
}

void CWeaponEngyDestroy::StopEffects(bool stopSound) {
	// Turn off our effect state
	DoEffect(EFFECT_NONE);

#ifndef CLIENT_DLL
	//Shut off sounds
	if (stopSound && GetMotorSound() != NULL) {
		(CSoundEnvelopeController::GetController()).SoundFadeOut( GetMotorSound(), 0.1f );
	}
#endif
}

void CWeaponEngyDestroy::StartEffects(void) {
#ifdef CLIENT_DLL

	// ------------------------------------------
	// Core
	// ------------------------------------------

	if ( m_Parameters[PHYSCANNON_CORE].GetMaterial() == NULL ) {
		m_Parameters[PHYSCANNON_CORE].GetScale().Init( 0.0f, 1.0f, 0.1f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().Init( 255.0f, 255.0f, 0.1f );
		m_Parameters[PHYSCANNON_CORE].SetAttachment( 1 );
		
		if ( m_Parameters[PHYSCANNON_CORE].SetMaterial( PHYSCANNON_CENTER_GLOW ) == false ) {
			// This means the texture was not found
			Assert( 0 );
		}
	}

	// ------------------------------------------
	// Blast
	// ------------------------------------------

	if ( m_Parameters[PHYSCANNON_BLAST].GetMaterial() == NULL ) {
		m_Parameters[PHYSCANNON_BLAST].GetScale().Init( 0.0f, 1.0f, 0.1f );
		m_Parameters[PHYSCANNON_BLAST].GetAlpha().Init( 255.0f, 255.0f, 0.1f );
		m_Parameters[PHYSCANNON_BLAST].SetAttachment( 1 );
		m_Parameters[PHYSCANNON_BLAST].SetVisible( false );
		
		if ( m_Parameters[PHYSCANNON_BLAST].SetMaterial( PHYSCANNON_BLAST_SPRITE ) == false ) {
			// This means the texture was not found
			Assert( 0 );
		}
	}

	// ------------------------------------------
	// Glows
	// ------------------------------------------

	const char *attachNamesGlowThirdPerson[NUM_GLOW_SPRITES] = 
	{
		"fork1m",
		"fork1t",
		"fork2m",
		"fork2t",
		"fork3m",
		"fork3t",
	};

	const char *attachNamesGlow[NUM_GLOW_SPRITES] = 
	{
		"fork1b",
		"fork1m",
		"fork1t",
		"fork2b",
		"fork2m",
		"fork2t"
	};

	//Create the glow sprites
	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ ) {
		if ( m_Parameters[i].GetMaterial() != NULL )
			continue;

		m_Parameters[i].GetScale().SetAbsolute( 0.05f * SPRITE_SCALE );
		m_Parameters[i].GetAlpha().SetAbsolute( 64.0f );
		
		// Different for different views
		if ( ShouldDrawUsingViewModel() ) {
			m_Parameters[i].SetAttachment( LookupAttachment( attachNamesGlow[i-PHYSCANNON_GLOW1] ) );
		} else {
			m_Parameters[i].SetAttachment( LookupAttachment( attachNamesGlowThirdPerson[i-PHYSCANNON_GLOW1] ) );
		}
		m_Parameters[i].SetColor( Vector( 10, 128, 200 ) );
		
		if ( m_Parameters[i].SetMaterial( PHYSCANNON_GLOW_SPRITE ) == false ) {
			// This means the texture was not found
			Assert( 0 );
		}
	}

	// ------------------------------------------
	// End caps
	// ------------------------------------------

	const char *attachNamesEndCap[NUM_ENDCAP_SPRITES] = 
	{
		"fork1t",
		"fork2t",
		"fork3t"
	};
	
	//Create the glow sprites
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ ) {
		if ( m_Parameters[i].GetMaterial() != NULL )
			continue;

		m_Parameters[i].GetScale().SetAbsolute( 0.05f * SPRITE_SCALE );
		m_Parameters[i].GetAlpha().SetAbsolute( 255.0f );
		m_Parameters[i].SetAttachment( LookupAttachment( attachNamesEndCap[i-PHYSCANNON_ENDCAP1] ) );
		m_Parameters[i].SetVisible( false );
		
		if ( m_Parameters[i].SetMaterial( PHYSCANNON_ENDCAP_SPRITE ) == false ) {
			// This means the texture was not found
			Assert( 0 );
		}
	}

#endif
}

void CWeaponEngyDestroy::DoEffectClosed(void) {
#ifdef CLIENT_DLL
	if (!ShouldDrawUsingViewModel())
		return;

	// Turn off the end-caps
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}
	
#endif

}

void CWeaponEngyDestroy::DoEffectReady(void) {
#ifdef CLIENT_DLL
	if (!ShouldDrawUsingViewModel())
		return;

	// Special POV case
	if ( ShouldDrawUsingViewModel() )
	{
		//Turn on the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 14.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 128.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].SetVisible();
	}
	else
	{
		//Turn off the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 8.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 0.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].SetVisible();
	}

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	{
		m_Parameters[i].GetScale().InitFromCurrent( 0.4f * SPRITE_SCALE, 0.2f );
		m_Parameters[i].GetAlpha().InitFromCurrent( 64.0f, 0.2f );
		m_Parameters[i].SetVisible();
	}

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

#endif

}

void CWeaponEngyDestroy::DoEffectHolding(void) {
#ifdef CLIENT_DLL

	if ( ShouldDrawUsingViewModel() )
	{
		// Scale up the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 16.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 255.0f, 0.1f );
		m_Parameters[PHYSCANNON_CORE].SetVisible();

		// Prepare for scale up
		m_Parameters[PHYSCANNON_BLAST].SetVisible( false );

		// Turn on the glow sprites
		for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
		{
			m_Parameters[i].GetScale().InitFromCurrent( 0.5f * SPRITE_SCALE, 0.2f );
			m_Parameters[i].GetAlpha().InitFromCurrent( 64.0f, 0.2f );
			m_Parameters[i].SetVisible();
		}

		// Turn on the glow sprites
		// NOTE: The last glow is left off for first-person
		for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES-1); i++ )
		{
			m_Parameters[i].SetVisible();
		}

		// Create our beams
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		CBaseEntity *pBeamEnt = pOwner->GetViewModel();

		// Setup the beams
		m_Beams[0].Init( LookupAttachment( "fork1t" ), 1, pBeamEnt, true );
		m_Beams[1].Init( LookupAttachment( "fork2t" ), 1, pBeamEnt, true );

		// Set them visible
		m_Beams[0].SetVisible();
		m_Beams[1].SetVisible();
	}
	//else
	//{
	//	// Scale up the center sprite
	//	m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 14.0f, 0.2f );
	//	m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 255.0f, 0.1f );
	//	m_Parameters[PHYSCANNON_CORE].SetVisible();

	//	// Prepare for scale up
	//	m_Parameters[PHYSCANNON_BLAST].SetVisible( false );

	//	// Turn on the glow sprites
	//	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	//	{
	//		m_Parameters[i].GetScale().InitFromCurrent( 0.5f * SPRITE_SCALE, 0.2f );
	//		m_Parameters[i].GetAlpha().InitFromCurrent( 64.0f, 0.2f );
	//		m_Parameters[i].SetVisible();
	//	}

	//	// Turn on the glow sprites
	//	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	//	{
	//		m_Parameters[i].SetVisible();
	//	}

	//	// Setup the beams
	//	m_Beams[0].Init( LookupAttachment( "fork1t" ), 1, this, false );
	//	m_Beams[1].Init( LookupAttachment( "fork2t" ), 1, this, false );
	//	m_Beams[2].Init( LookupAttachment( "fork3t" ), 1, this, false );

	//	// Set them visible
	//	m_Beams[0].SetVisible();
	//	m_Beams[1].SetVisible();
	//	m_Beams[2].SetVisible();
	//}

#endif
}


//-----------------------------------------------------------------------------
// Launch effects
//-----------------------------------------------------------------------------
void CWeaponEngyDestroy::DoEffectLaunch(Vector *pos) {
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	Vector	endPos;
	Vector	shotDir;

	// See if we need to predict this position
	if ( pos == NULL ) {
		// Hit an entity if we're holding one
		//if ( m_hAttachedObject ) {
		//	endPos = m_hAttachedObject->WorldSpaceCenter();
		//	
		//	shotDir = endPos - pOwner->Weapon_ShootPosition();
		//	VectorNormalize( shotDir );
		//} else 
		{
			// Otherwise try and find the right spot
			endPos = pOwner->Weapon_ShootPosition();
			pOwner->EyeVectors( &shotDir );

			trace_t	tr;
			UTIL_TraceLine
			(
				endPos,
				endPos + ( shotDir * MAX_TRACE_LENGTH ),
				MASK_SHOT,
				pOwner,
				COLLISION_GROUP_NONE,
				&tr
			);
			
			endPos = tr.endpos;
			shotDir = endPos - pOwner->Weapon_ShootPosition();
			VectorNormalize( shotDir );
		}
	} else {
		// Use what is supplied
		endPos = *pos;
		shotDir = ( endPos - pOwner->Weapon_ShootPosition() );
		VectorNormalize( shotDir );
	}

	// End hit
	CPVSFilter filter( endPos );

	// Don't send this to the owning player, they already had it predicted
	if (IsPredicted()) {
		filter.UsePredictionRules();
	}

	// Do an impact hit
	CEffectData	data;
	data.m_vOrigin = endPos;
#ifdef CLIENT_DLL
	data.m_hEntity = GetRefEHandle();
#else
	data.m_nEntIndex = entindex();
#endif

	te->DispatchEffect( filter, 0.0, data.m_vOrigin, "PhyscannonImpact", data );

#ifdef CLIENT_DLL

	if (!ShouldDrawUsingViewModel()) {
		//Turn on the blast sprite and scale
		m_Parameters[PHYSCANNON_BLAST].GetScale().Init( 8.0f, 64.0f, 0.1f );
		m_Parameters[PHYSCANNON_BLAST].GetAlpha().Init( 255.0f, 0.0f, 0.2f );
		m_Parameters[PHYSCANNON_BLAST].SetVisible();
	}

#endif

}

void CWeaponEngyDestroy::DoEffectNone(void) {
#ifdef CLIENT_DLL

	//if (!ShouldDrawUsingViewModel())
	//	return;

	//Turn off main glows
	m_Parameters[PHYSCANNON_CORE].SetVisible( false );
	m_Parameters[PHYSCANNON_BLAST].SetVisible( false );

	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

	m_Beams[0].SetVisible( false );
	m_Beams[1].SetVisible( false );
	m_Beams[2].SetVisible( false );

#endif
}

void CWeaponEngyDestroy::DoEffect(int effectType, Vector *pos) {
	m_EffectState = effectType;

#ifdef CLIENT_DLL
	// Save predicted state
	m_nOldEffectState = m_EffectState;
#endif

	switch( effectType )
	{
	case EFFECT_CLOSED:
		DoEffectClosed();
		break;

	case EFFECT_READY:
		DoEffectReady();
		break;

	case EFFECT_HOLDING:
		DoEffectHolding();
		break;

	case EFFECT_LAUNCH:
		DoEffectLaunch( pos );
		break;

	default:
	case EFFECT_NONE:
		DoEffectNone();
		break;
	}
}

#ifdef CLIENT_DLL

extern void FormatViewModelAttachment(Vector &vOrigin, bool bInverse);

void CWeaponEngyDestroy::GetEffectParameters(EffectType_t effectID, color32 &color, float &scale, IMaterial **pMaterial, Vector &vecAttachment) {
	const float dt = gpGlobals->curtime;

	// Get alpha
	float alpha = m_Parameters[effectID].GetAlpha().Interp( dt );
	
	// Get scale
	scale = m_Parameters[effectID].GetScale().Interp( dt );
	
	// Get material
	*pMaterial = (IMaterial *) m_Parameters[effectID].GetMaterial();

	// Setup the color
	color.r = (int) m_Parameters[effectID].GetColor().x;
	color.g = (int) m_Parameters[effectID].GetColor().y;
	color.b = (int) m_Parameters[effectID].GetColor().z;
	color.a = (int) alpha;

	// Setup the attachment
	int		attachment = m_Parameters[effectID].GetAttachment();
	QAngle	angles;

	// Format for first-person
	if ( ShouldDrawUsingViewModel() ) {
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		
		if (pOwner) {
			pOwner->GetViewModel()->GetAttachment( attachment, vecAttachment, angles );
			::FormatViewModelAttachment( vecAttachment, true );
		}
	} else {
		GetAttachment( attachment, vecAttachment, angles );
	}
}

bool CWeaponEngyDestroy::IsEffectVisible(EffectType_t effectID) {
	return m_Parameters[effectID].IsVisible();
}

void CWeaponEngyDestroy::DrawEffectSprite(EffectType_t effectID) {
	color32 color;
	float scale;
	IMaterial *pMaterial;
	Vector	vecAttachment;

	// Don't draw invisible effects
	if ( IsEffectVisible( effectID ) == false )
		return;

	// Get all of our parameters
	GetEffectParameters( effectID, color, scale, &pMaterial, vecAttachment );

	// Msg( "Scale: %.2f\tAlpha: %.2f\n", scale, alpha );

	// Don't render fully translucent objects
	if ( color.a <= 0.0f )
		return;

	// Draw the sprite
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( pMaterial, this );
	DrawSprite( vecAttachment, scale, scale, color );
}

void CWeaponEngyDestroy::DrawEffects(void) {
	// Draw the core effects
	DrawEffectSprite( PHYSCANNON_CORE );
	DrawEffectSprite( PHYSCANNON_BLAST );
	
	// Draw the glows
	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ ) {
		DrawEffectSprite( (EffectType_t) i );
	}

	// Draw the endcaps
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ ) {
		DrawEffectSprite( (EffectType_t) i );
	}
}

int CWeaponEngyDestroy::DrawModel(int flags) {
	if (flags & STUDIO_TRANSPARENCY) {
		DrawEffects();
		return 1;
	}

	// Only do this on the opaque pass
	return BaseClass::DrawModel( flags );
}

void CWeaponEngyDestroy::ViewModelDrawn( C_BaseViewModel *pBaseViewModel ) {
	DrawEffects();
	BaseClass::ViewModelDrawn( pBaseViewModel );
}

bool CWeaponEngyDestroy::IsTransparent(void) {
	return true;
}


void CWeaponEngyDestroy::NotifyShouldTransmit(ShouldTransmitState_t state) {
	BaseClass::NotifyShouldTransmit(state);

	if (state == SHOULDTRANSMIT_END) {
		DoEffect(EFFECT_NONE);
	}
}

#endif