//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "gamestats.h"
#include "class_info.h"
#include "item_info.h"
#include "viewport_panel_names.h"
#include "ents/egg.h"
#include "grenade_guardian.h"
#include "weapon_c4.h"
#include "grenade_c4.h"
#include "ammodef.h"
#include "ents/pack_item.h"
#include "ents/ent_infested_corpse.h"
#include "weapon_drone_slash.h"
#include "particle_parse.h"
#include "weapon_engy.h"
#include "weapon_breeder.h"
#include "grenade_guardian.h"
#include "grenade_smk.h"
#include "grenade_frag.h"
#include "grenade_acid.h"
#include "usermessages.h"
#include "weapon_plasma_canon.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"


#define GRENADE_RADIUS	4.0f
#define GRENADE_THROW_INTERVAL 1.5f

#define DRONE_SPIT_RECOVER_TIME	1.0f
int g_iLastCitizenModel = 0;
int g_iLastCombineModel = 0;

CBaseEntity	 *g_pLastSpiderSpawn = NULL;
CBaseEntity	 *g_pLastHumanSpawn = NULL;
extern CBaseEntity				*g_pLastSpawn;

#define HL2MP_COMMAND_MAX_RATE 0.3

#define GUARDIAN_INFESTATION_INTERVAL 30.0f
#define GUARDIAN_MAX_INFESTED 15

#define GUARDIAN_ARMOR_CTX "guardArmorRecharge"
#define GUARDIAN_ARMOR_RECHARGE_INT 5.0f

#define GUARDIAN_ATTACK_MOTION_CTX "guardAttackMotion"
#define GUARDIAN_ATTACK_MOTION_INT 1.5f

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );
void DropPrimedSmkGrenGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

#define JETPACK_PARTICLES "jetpack_puff"
#define JETPACK_SPARK "sparking_gear"

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

//LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
//LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 5 ),
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	SendPropInt( SENDINFO( grenade_type ), 6 ),
	
	SendPropInt( SENDINFO( m_iClassNumber), 5),
	SendPropBool( SENDINFO( stopped) ),
	SendPropBool( SENDINFO( plasma_ready ) ),
	SendPropBool( SENDINFO( jetpack_on ) ),
	SendPropInt( SENDINFO( pack_item_0 ), 4 ),
	SendPropInt( SENDINFO( pack_item_1 ), 4 ),
	SendPropInt( SENDINFO( pack_item_2 ), 4 ),
	SendPropInt( SENDINFO( pack_item_idx ), 4 ),
	SendPropBool( SENDINFO( powerArmorEnabled ) ),
	SendPropBool( SENDINFO( attackMotion ) ),
	SendPropBool( SENDINFO( bugGlow ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
	DEFINE_THINKFUNC(MovementThink),
	DEFINE_THINKFUNC(RechargeThink),
	DEFINE_THINKFUNC(JetIgniteThink),
	DEFINE_THINKFUNC(JetOnThink),
END_DATADESC()

#define MAX_COMBINE_MODELS 4
#define MODEL_CHANGE_INTERVAL 5.0f
#define TEAM_CHANGE_INTERVAL 5.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

extern HL2MPViewVectors g_HL2MPViewVectors;

#define PLASMA_THINK_CTX			"plasma_think"
#define PLASMA_RECHARGE_INTERVAL	0.15f
#define PLASMA_RECHARGE_DELAY		2.5f
#define PLASMA_RECHARGE_MAX_AMT		25

#define HINT_THINK_CTX				"hint_think_ctx"
#define HINT_THINK_DELAY			1.0f

#define JET_IGNITE_CTX				"jet_ignite_think"
#define JET_IGNITE_DELAY			0.2f

#define JET_ON_CTX					"jet_on_think"
#define JET_ON_INTERVAL				0.1f
#define EXTERM_JETBURST_SOUND		"Exterm.JetBurst"
#define JET_BURST_MIN_PLASMA		25

#define GUARDIAN_SWALLOW_SND		"Guardian.SwallowC4"

#define HATCHY_SLASH_SOUND			"Hatchy.Slash"

#define XT_PLASMA_HIT				"XT.PlasmaHit"

#define PLASMA_SHIELD_ON			"PlasmaShield.On"
#define PLASMA_SHIELD_OFF			"PlasmaShield.Off"

#define PLASMA_SHIELD_CTX			"plasma_shield_ctx"

ConVar dk_team_balance( "dk_team_balance", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "if enabled, players won't be allowed to join a team with too many players" );

CHL2MP_Player::CHL2MP_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;
	m_iPlayerPoints = 0;
#ifdef DEBUG
	m_iPlayerPoints = 10;
#endif
	m_iClassNumber = -1;

	chose_class = false;
	choosing_class = false;

	next_ammo_pickup = 0.0f;

	classVectors = &g_HL2MPViewVectors;
	next_heal_sound = 0.0f;
	dropC4 = false;
	num_hints = 0;

	plasma_ammo_type = GetAmmoDef()->Index("plasma");
	jetpack_on = false;
	jet_start = FLT_MAX;
	next_infestation = 0.0f;
	taunt_sound = NULL;
	pri_weapon = NULL;
	sec_weapon = NULL;
	gren_weapon = NULL;
	spec_weapon = NULL;
	attackMotion = false;
	powerArmorEnabled = true;

	RegisterThinkContext(DRONE_SPIT_CTX);
	RegisterThinkContext(PLASMA_THINK_CTX);
	RegisterThinkContext(JET_IGNITE_CTX);
	RegisterThinkContext(GUARDIAN_ARMOR_CTX);
	RegisterThinkContext(GUARDIAN_ATTACK_MOTION_CTX);
	RegisterThinkContext(HINT_THINK_CTX);
	RegisterThinkContext(PLASMA_SHIELD_CTX);

	BaseClass::ChangeTeam( 0 );
}

void CHL2MP_Player::SwallowC4Sound(void) {
	EmitSound(GUARDIAN_SWALLOW_SND);
}

void CHL2MP_Player::PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper) {
	if (GetTeamNumber() == TEAM_HUMANS &&
		m_iClassNumber == CLASS_EXTERMINATOR_IDX &&
		!IsDead()) {

			if (ucmd->buttons & IN_JUMP) {
				if (!(m_Local.m_nOldButtons & IN_JUMP)) {
					StartJetPack();
				}
			} else {
				if (m_Local.m_nOldButtons & IN_JUMP) {
					StopJetPack();
				}
			}

	} 
	//else if (GetTeamNumber() == TEAM_SPIDERS &&
	//	m_iClassNumber == CLASS_GUARDIAN_IDX &&
	//	!IsDead()) {

	//		if (ucmd->buttons & IN_ATTACK2) {
	//			if (!(m_Local.m_nOldButtons & IN_ATTACK2)) {
	//				if (next_infestation < gpGlobals->curtime) {
	//					InfestCorpse();
	//				}
	//			}
	//		}
	//}

	BaseClass::PlayerRunCommand(ucmd, moveHelper);
}

void CHL2MP_Player::NoAttackMotion(void) {
	attackMotion = false;
}

void CHL2MP_Player::AttackMotion(void) {
	attackMotion = true;

	SetContextThink(&CHL2MP_Player::NoAttackMotion, gpGlobals->curtime + GUARDIAN_ATTACK_MOTION_INT, GUARDIAN_ATTACK_MOTION_CTX);
}

void CHL2MP_Player::GuardianArmorRechargeThink(void) {
	int maxArmor;

	maxArmor = GetMaxArmor();

	if (IsStopped()) {
		if (ArmorValue() < maxArmor) {
			IncrementArmorValue(1, maxArmor);
			SetNextThink(gpGlobals->curtime + GUARDIAN_ARMOR_RECHARGE_INT, GUARDIAN_ARMOR_CTX);
		}
	}
}

void CHL2MP_Player::ResetGuardianArmorRecharge(void) {
	if (GetTeamNumber() == TEAM_SPIDERS &&
		m_iClassNumber == CLASS_GUARDIAN_IDX) {
			
			SetContextThink(&CHL2MP_Player::GuardianArmorRechargeThink, gpGlobals->curtime + GUARDIAN_ARMOR_RECHARGE_INT, GUARDIAN_ARMOR_CTX);

	}
}

void CHL2MP_Player::InfestCorpse(void) {
	CInfestedCorpse *c;

	next_infestation = gpGlobals->curtime + GUARDIAN_INFESTATION_INTERVAL;

	c = (CInfestedCorpse *)CreateEntityByName("ent_infested_corpse");
	if (c) {
		c->SetAbsOrigin(GetAbsOrigin() + Vector(0,0,7));
		c->SetAbsAngles(GetAbsAngles());
		c->SetCreator(this);
		DispatchSpawn(c);
	}
}

void CHL2MP_Player::StartJetPack(void) {
	SetContextThink(&CHL2MP_Player::JetIgniteThink, gpGlobals->curtime + JET_IGNITE_DELAY, JET_IGNITE_CTX);
}

void CHL2MP_Player::StopJetPack(void) {
	jetpack_on = false;
	SetContextThink(NULL, 0, JET_IGNITE_CTX);
	SetContextThink(NULL, 0, JET_ON_CTX);
	RegularRecharge();
	StopParticleEffects(this);
}

void CHL2MP_Player::JetIgniteThink(void) {
	if (PlasmaReady()) {
		jetpack_on = true;
		SetContextThink(&CHL2MP_Player::JetOnThink, gpGlobals->curtime + JET_ON_INTERVAL, JET_ON_CTX);

		CDisablePredictionFiltering foo2;
		DispatchParticleEffect(JETPACK_PARTICLES, GetAbsOrigin() + Vector(0,0,32), GetAbsAngles(), this);
	}
}

void CHL2MP_Player::JetOnThink(void) {
	// make jet sound
	// subtract plasma

	Vector origin;
	CSoundParameters params;
	CRecipientFilter filter;
	EmitSound_t ep;

	if (PlasmaReady() && GetAmmoCount(plasma_ammo_type) >= JET_BURST_MIN_PLASMA) {

		if (GetParametersForSound( EXTERM_JETBURST_SOUND, params, NULL )) {
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

		CDisablePredictionFiltering foo2;
		DispatchParticleEffect(JETPACK_PARTICLES, GetAbsOrigin() + Vector(0,0,32), GetAbsAngles(), this);

		RemoveAmmo(JET_BURST_MIN_PLASMA, plasma_ammo_type);

		SetNextThink(gpGlobals->curtime + JET_ON_INTERVAL, JET_ON_CTX);

		SendPowerArmorUpdate();

	} else {
		StopJetPack();
	}

	if (GetAmmoCount(plasma_ammo_type) < JET_BURST_MIN_PLASMA) {
		StopJetPack();
		DisablePlasma(false);
	}

}

bool CHL2MP_Player::JetOn(void) {
	return jetpack_on;
}

void CHL2MP_Player::RegularRecharge(void) {
	//
	// DisablePlasma will set a delay for recharge
	// and more calls to RegularRecharge shouldn't
	// interrupt that...
	//
	// so don't interrupt the recharger/recharge delay
	// if plasma is not ready (only happens after delay)
	//
	if (plasma_ready == true) {
		recharge_mul = 0;
		recharge_start = gpGlobals->curtime;
		SetContextThink(&CHL2MP_Player::RechargeThink, gpGlobals->curtime + PLASMA_RECHARGE_DELAY, PLASMA_THINK_CTX);
	}

	SendPowerArmorUpdate();
}

void CHL2MP_Player::PlasmaOn(void) {
	// the plasma rifle ammo is the default plasma amount
	plasma_ready = true;
	plasma_recovering = false;
}

void CHL2MP_Player::PlasmaOff(void) {
	SetContextThink(NULL, 0, PLASMA_THINK_CTX);
	plasma_ready = false;
	StopParticleEffects(this);
}

bool CHL2MP_Player::PlasmaReady(void) {
	return plasma_ready;
}

void CHL2MP_Player::RechargeThink(void) {
	int x;
	int actual;

	if (jetpack_on == false) {

		x = 1.5f * (gpGlobals->curtime - recharge_start);
		x = clamp(x, 0, PLASMA_RECHARGE_MAX_AMT);

		CBasePlayer::GiveAmmo(x, plasma_ammo_type, true);

		actual = GetAmmoCount(plasma_ammo_type);

		if (actual > max_plasma) {
			//
			// Engy can recharge past max_plasma if we're not careful
			//
			CBasePlayer::RemoveAmmo(actual - max_plasma, plasma_ammo_type);
		}

		SendPowerArmorUpdate();

		if (GetAmmoCount(plasma_ammo_type) < max_plasma) {
			SetNextThink(gpGlobals->curtime + PLASMA_RECHARGE_INTERVAL, PLASMA_THINK_CTX);
		
			if (plasma_recovering == false)
				plasma_ready = true;

			return;
		}
	
		if (plasma_recovering) {
			plasma_recovering = false;
		}

		plasma_ready = true;

	}
}

// dmg indicates extended recovery delay
void CHL2MP_Player::DisablePlasma(bool dmg) {
	SetAmmoCount(0, plasma_ammo_type);
	plasma_ready = false;
	if (dmg) {
		SetContextThink(&CHL2MP_Player::RechargeThink, gpGlobals->curtime + PLASMA_RECHARGE_DELAY * 10.0f, PLASMA_THINK_CTX);
		plasma_recovering = true;
	} else {
		SetContextThink(&CHL2MP_Player::RechargeThink, gpGlobals->curtime + PLASMA_RECHARGE_DELAY * 3.0f, PLASMA_THINK_CTX);
	}
	
	SendPowerArmorUpdate();

	EmitSound(PLASMA_SHIELD_OFF);
}

//
// let plasma shield absorb damage by modifying the info's Damage
// but only if the plasma shield is ready..
//
void CHL2MP_Player::FilterDamage(CTakeDamageInfo &info) {
	int initial_plasma;
	float dmg;
	color32 blue = {0,0,255,70};

	initial_plasma = PlasmaCharge();
	dmg = info.GetDamage();

	if (PlasmaReady() && powerArmorEnabled) {
		if (dmg > initial_plasma) {
			//
			// shields will be completely drained by the hit
			// 
			
			info.SetDamage(dmg - initial_plasma);
			BurnPlasma(dmg);
			
		} else {
			//
			// shields absorbed the damage alright, so begin recharge think
			// after a short delay
			//
			initial_plasma -= dmg;
			info.SetDamage(0);
			BurnPlasma(dmg);
		}

		//
		// play a sound, do a view punch
		//

		EmitSound(XT_PLASMA_HIT);
		
		UTIL_ScreenFade( this, blue, 0.2, 0.3, FFADE_MODULATE );

		m_Local.m_vecPunchAngle.SetX( RandomFloat( -3, -10 ) );

		CDisablePredictionFiltering foo;

		DispatchParticleEffect(JETPACK_SPARK, GetAbsOrigin() + Vector(0,0,32), GetAbsAngles(), this);
	}
}

int CHL2MP_Player::PlasmaCharge(void) {
	return GetAmmoCount(plasma_ammo_type);
}

void CHL2MP_Player::PlasmaShot(void) {
	if (plasma_ready == false)
		return;

	RemoveAmmo(1, plasma_ammo_type);

	if (GetAmmoCount(plasma_ammo_type) <= 0) {
		DisablePlasma(false);
	} else {
		RegularRecharge();
	}

}

//
// railgun does damage to plasma shield
//
void CHL2MP_Player::RailgunShot(void) {
	if (plasma_ready == false)
		return;

	DisablePlasma(true);

	max_plasma -= 75;
	max_plasma = clamp(max_plasma, 0, 250);
}

void CHL2MP_Player::BurnPlasma(int amt) {
	if (plasma_ready == false)
		return;

	RemoveAmmo(amt, plasma_ammo_type);

	if (GetAmmoCount(plasma_ammo_type) <= 0) {
		DisablePlasma(false);
	} else {
		RegularRecharge();
	}
}

void CHL2MP_Player::SendPowerArmorUpdate(void) {
	int val;

	if (powerArmorEnabled)
		val = PlasmaCharge();
	else
		val = 0;

	if (GetTeamNumber() == TEAM_HUMANS &&
		(m_iClassNumber == CLASS_ENGINEER_IDX ||
		m_iClassNumber == CLASS_EXTERMINATOR_IDX)) {

		CSingleUserRecipientFilter user(this);
		user.MakeReliable();

		if (usermessages->LookupUserMessage( "Battery" ) != -1) {
			UserMessageBegin(user, "Battery" );
					WRITE_SHORT(val);
			MessageEnd();
		}

	}
}

void CHL2MP_Player::TogglePowerArmor(void) {
	
	if (GetTeamNumber() == TEAM_HUMANS &&
		(m_iClassNumber == CLASS_ENGINEER_IDX ||
		m_iClassNumber == CLASS_EXTERMINATOR_IDX)) {

		CDisablePredictionFiltering foo;

		if (powerArmorEnabled) {
			EmitSound(PLASMA_SHIELD_OFF);
			powerArmorEnabled = false;
		} else {
			EmitSound(PLASMA_SHIELD_ON);
			powerArmorEnabled = true;
		}

		SendPowerArmorUpdate();
	}
}

void CHL2MP_Player::AddPlayerPoints(int i) {
	m_iPlayerPoints += i;
	if (m_iPlayerPoints > MAX_PLAYER_POINTS)
		m_iPlayerPoints = MAX_PLAYER_POINTS;

	CSingleUserRecipientFilter user(this);
	UserMessageBegin(user, "Points");
		WRITE_BYTE((unsigned char)m_iPlayerPoints);
	MessageEnd();
}

void CHL2MP_Player::SubtractPlayerPoints(int i) {
	m_iPlayerPoints -= i;
	if (m_iPlayerPoints < 0)
		m_iPlayerPoints = 0;

	CSingleUserRecipientFilter user(this);
	UserMessageBegin(user, "Points");
		WRITE_BYTE((unsigned char)m_iPlayerPoints);
	MessageEnd();
}

void CHL2MP_Player::DropC4OnHit(bool value) {
	dropC4 = value;
}

CHL2MP_Player::~CHL2MP_Player( void )
{

}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache( void )
{
	int i;

	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );
	for (i = 0; i < NUM_SPIDER_CLASSES; i++)
		PrecacheModel(dk_spider_classes[i].model);

	for (i = 0; i < NUM_HUMAN_CLASSES; i++)
		PrecacheModel(dk_human_classes[i].model);

	PrecacheFootStepSounds();

	PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" );
	PrecacheScriptSound(EXTERM_JETBURST_SOUND);
	PrecacheScriptSound(GUARDIAN_SWALLOW_SND);
	PrecacheParticleSystem(JETPACK_PARTICLES);
	PrecacheParticleSystem(JETPACK_SPARK);
	PrecacheScriptSound(HATCHY_SLASH_SOUND);
	PrecacheScriptSound(XT_PLASMA_HIT);
	PrecacheScriptSound(PLASMA_SHIELD_ON);
	PrecacheScriptSound(PLASMA_SHIELD_OFF);

	// may need to precache more stuff here (bug sounds, etc)

	// taunts
	PrecacheScriptSound(ENGY_TAUNT);
	PrecacheScriptSound(GRUNT_TAUNT);
	PrecacheScriptSound(SHOCK_TAUNT);
	PrecacheScriptSound(HEAVY_TAUNT);
	PrecacheScriptSound(COMMANDO_TAUNT);
	PrecacheScriptSound(EXTERM_TAUNT);
	PrecacheScriptSound(MECH_TAUNT);

	PrecacheScriptSound(BREEDER_TAUNT);
	PrecacheScriptSound(HATCHY_TAUNT);
	PrecacheScriptSound(DRONE_TAUNT);
	PrecacheScriptSound(STINGER_TAUNT);
	PrecacheScriptSound(GUARDIAN_TAUNT);
	PrecacheScriptSound(STALKER_TAUNT);

}

void CHL2MP_Player::GiveAllItems( void )
{
	EquipSuit();

	CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 255,	"AR2" );
	CBasePlayer::GiveAmmo( 5,	"AR2AltFire" );
	CBasePlayer::GiveAmmo( 255,	"SMG1");
	CBasePlayer::GiveAmmo( 1,	"smg1_grenade");
	CBasePlayer::GiveAmmo( 255,	"Buckshot");
	CBasePlayer::GiveAmmo( 32,	"357" );
	CBasePlayer::GiveAmmo( 3,	"rpg_round");

	CBasePlayer::GiveAmmo( 1,	"grenade" );
	CBasePlayer::GiveAmmo( 2,	"slam" );

	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_stunstick" );
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_357" );

	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem( "weapon_ar2" );
	
	GiveNamedItem( "weapon_shotgun" );
	GiveNamedItem( "weapon_frag" );
	
	GiveNamedItem( "weapon_crossbow" );
	
	GiveNamedItem( "weapon_rpg" );

	GiveNamedItem( "weapon_slam" );

	GiveNamedItem( "weapon_physcannon" );
	
}

//
// small = engy fix hit
// not small = ammo crate/etc
//
void CHL2MP_Player::RefilAmmo(bool small) {
	int amt;

	//
	// will need logic for ammo vs weapon
	// will also need ammo for not giving super weapons back
	//

	switch(GetTeamNumber()) {
	case TEAM_SPIDERS:
		if (Q_strcmp(dk_spider_classes[m_iClassNumber].pri_ammo, AMMO_NULL))
			CBasePlayer::GiveAmmo(999, dk_spider_classes[m_iClassNumber].pri_ammo);

		if (Q_strcmp(dk_spider_classes[m_iClassNumber].sec_ammo, AMMO_NULL))
			CBasePlayer::GiveAmmo(999, dk_spider_classes[m_iClassNumber].sec_ammo);

		if (Q_strcmp(dk_spider_classes[m_iClassNumber].grenade_type, GRENADE_NULL))
			CBasePlayer::GiveAmmo(dk_spider_classes[m_iClassNumber].max_grenades, dk_spider_classes[m_iClassNumber].grenade_type);

		break;

	case TEAM_HUMANS:
		if (small == false) {
			if (m_iClassNumber != CLASS_EXTERMINATOR_IDX) { // plasma isn't given

				if (m_iClassNumber == CLASS_MECH_IDX) {
					// mech ammo is hard for the little crate to make
					// encourage the use of engineers to refil plasma canons
					amt = 30;
				} else {
					amt = 200;
				}

				if (Q_strcmp(dk_human_classes[m_iClassNumber].pri_ammo, AMMO_NULL))
					CBasePlayer::GiveAmmo(amt, dk_human_classes[m_iClassNumber].pri_ammo);
			}

			if (Q_strcmp(dk_human_classes[m_iClassNumber].sec_ammo, AMMO_NULL))
				CBasePlayer::GiveAmmo(999, dk_human_classes[m_iClassNumber].sec_ammo);

			if (Q_strcmp(dk_human_classes[m_iClassNumber].grenade_type, GRENADE_NULL))
				CBasePlayer::GiveAmmo(dk_human_classes[m_iClassNumber].max_grenades, dk_human_classes[m_iClassNumber].grenade_type);

			// shock trooper's extra shells are special :)
			if (m_iClassNumber == CLASS_SHOCK_IDX) {
				if (Q_strcmp(dk_human_classes[m_iClassNumber].pri_ammo, AMMO_NULL))
					CBasePlayer::GiveAmmo(3, "xp_shells");
			}

		} else {
			
			//
			// All human classes can gain secondary ammo from engy fixes
			//
			// Exterms can't get primary (recharge-based)
			// Shock won't get XP shells (ammo crate only)
			//
			// nobody gets grenades from engy
			//
			if (Q_strcmp(dk_human_classes[m_iClassNumber].sec_ammo, AMMO_NULL))
				CBasePlayer::GiveAmmo(999, dk_human_classes[m_iClassNumber].sec_ammo);

			switch (m_iClassNumber) {
			case CLASS_GRUNT_IDX:
			case CLASS_COMMANDO_IDX:
				// give the grunt/cmndo about a clip at a time
				if (Q_strcmp(dk_human_classes[m_iClassNumber].pri_ammo, AMMO_NULL))
					CBasePlayer::GiveAmmo(25, dk_human_classes[m_iClassNumber].pri_ammo);

				break;

			case CLASS_SHOCK_IDX:
				// give shock 2 shells at a time
				if (Q_strcmp(dk_human_classes[m_iClassNumber].pri_ammo, AMMO_NULL))
					CBasePlayer::GiveAmmo(2, dk_human_classes[m_iClassNumber].pri_ammo);

				break;

			case CLASS_HEAVY_IDX:
				// give heavy 1 rocket at a time
				if (Q_strcmp(dk_human_classes[m_iClassNumber].pri_ammo, AMMO_NULL))
					CBasePlayer::GiveAmmo(1, dk_human_classes[m_iClassNumber].pri_ammo);

				break;

			case CLASS_MECH_IDX:
				// give the mech 10 bolts at a time
				if (Q_strcmp(dk_human_classes[m_iClassNumber].pri_ammo, AMMO_NULL))
					CBasePlayer::GiveAmmo(10, dk_human_classes[m_iClassNumber].pri_ammo);

				break;

			default:
				break;
			}
		}

		break;

	default:
		break;
	}
}

void CHL2MP_Player::GiveDefaultItems( void ) {
	EquipSuit();

	pack_item_0 = 0;
	pack_item_1 = 0;
	pack_item_2 = 0;
	pack_item_idx = -1;

	pri_weapon = NULL;
	sec_weapon = NULL;
	gren_weapon = NULL;
	spec_weapon = NULL;

	// give primary, secondary, grenades (and ammo)

	switch(GetTeamNumber()) {
	case TEAM_SPIDERS:

		if (Q_strcmp(dk_spider_classes[m_iClassNumber].sec_weapon, WEAPON_NULL)) {
			GiveNamedItem(dk_spider_classes[m_iClassNumber].sec_weapon);
			sec_weapon = Weapon_OwnsThisType(dk_spider_classes[m_iClassNumber].sec_weapon);
		}

		if (Q_strcmp(dk_spider_classes[m_iClassNumber].grenade_type, GRENADE_NULL)) {
			CBasePlayer::GiveAmmo(dk_spider_classes[m_iClassNumber].max_grenades, dk_spider_classes[m_iClassNumber].grenade_type);
		}
	
		if (Q_strcmp(dk_spider_classes[m_iClassNumber].grenade_weapon, WEAPON_NULL)) {
			GiveNamedItem(dk_spider_classes[m_iClassNumber].grenade_weapon);
			gren_weapon = Weapon_OwnsThisType(dk_spider_classes[m_iClassNumber].grenade_weapon);
		}

		if (Q_strcmp(dk_spider_classes[m_iClassNumber].pri_weapon, WEAPON_NULL)) {
			GiveNamedItem(dk_spider_classes[m_iClassNumber].pri_weapon);
			pri_weapon = Weapon_OwnsThisType(dk_spider_classes[m_iClassNumber].pri_weapon);
			Weapon_Switch(pri_weapon);
		}

		break;

	case TEAM_HUMANS:

		if (Q_strcmp(dk_human_classes[m_iClassNumber].sec_weapon, WEAPON_NULL)) {
			GiveNamedItem(dk_human_classes[m_iClassNumber].sec_weapon);
			sec_weapon = Weapon_OwnsThisType(dk_human_classes[m_iClassNumber].sec_weapon);
		}

		if (Q_strcmp(dk_human_classes[m_iClassNumber].grenade_type, GRENADE_NULL)) {
			CBasePlayer::GiveAmmo(dk_human_classes[m_iClassNumber].max_grenades, dk_human_classes[m_iClassNumber].grenade_type);
		}
		
		if (Q_strcmp(dk_human_classes[m_iClassNumber].grenade_weapon, WEAPON_NULL)) {
			GiveNamedItem(dk_human_classes[m_iClassNumber].grenade_weapon);
			gren_weapon = Weapon_OwnsThisType(dk_human_classes[m_iClassNumber].grenade_weapon);
		}

		// give commando C4
		if (m_iClassNumber == CLASS_COMMANDO_IDX) {
			//CBasePlayer::GiveAmmo(1, "grenade_c4");
			GiveNamedItem("weapon_c4");
			spec_weapon = Weapon_OwnsThisType("weapon_c4");
		}
		
		if (Q_strcmp(dk_human_classes[m_iClassNumber].pri_weapon, WEAPON_NULL)) {
			GiveNamedItem(dk_human_classes[m_iClassNumber].pri_weapon);
			pri_weapon = Weapon_OwnsThisType(dk_human_classes[m_iClassNumber].pri_weapon);
			Weapon_Switch(pri_weapon);
		}

		//if (m_iClassNumber == CLASS_EXTERMINATOR_IDX) {
		//	PlasmaOn();
		//}

		if (m_iClassNumber == CLASS_SHOCK_IDX) {
			CBasePlayer::GiveAmmo(3, "xp_shells");
		}

		pack_item_0 = PACK_ITEM_TYPE_HEALTH;
		pack_item_idx = 0;

		break;

	default:
		break;
	}
}

void CHL2MP_Player::PickDefaultSpawnTeam( void )
{
	if (GetTeamNumber() == 0) {
		ChangeTeam(TEAM_SPECTATOR);
	}
}

void CHL2MP_Player::DisableSpawnHint(void) {
	showSpawnHint = false;
}

bool CHL2MP_Player::ShowSpawnHint(void) {
	return showSpawnHint;
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{
	showSpawnHint = true;
	next_ammo_pickup = 0.0f;
	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;
	chose_class = false;
	classVectors = &g_HL2MPViewVectors;

	PickDefaultSpawnTeam();

	BaseClass::Spawn();

	//
	// commented out of player.cpp
	// parts overridden here, hopefully
	// the standing/crouch hull stuff is working
	//
	InitVCollision( GetAbsOrigin(), GetAbsVelocity() );
	
	grenade_type = -1;

	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );
		
		if (GetTeamNumber() == TEAM_SPIDERS ||
			GetTeamNumber() == TEAM_HUMANS) {

			GiveDefaultItems();

			SetThink(&CHL2MP_Player::MovementThink);
			SetNextThink(gpGlobals->curtime + 0.2f);

			SetPlayerClass(m_iClassNumber);
		}
	}

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	m_bReady = false;

	last_spit_hit = FLT_MAX;
	next_heal_sound = 0.0f;

	m_flNextGrenadeTime = 0.0f;
	

	inSpawnQueue = false;
	stopped = true;
	stoppedTicks = 0;
	dropC4 = false;
	powerArmorEnabled = false;

	next_infestation = 0.0f;
	attackMotion = false;

	CSingleUserRecipientFilter user(this);
	UserMessageBegin(user, "Points");
		WRITE_BYTE((unsigned char)m_iPlayerPoints);
	MessageEnd();


	if (!IsObserver() && IsAlive()) {
		if (num_hints < 3) {
			num_hints++;
			SetContextThink(&CHL2MP_Player::ShowHelpHint, gpGlobals->curtime + HINT_THINK_DELAY, HINT_THINK_CTX);
		}

	
		if (GetTeamNumber() == TEAM_HUMANS) {

			if (m_iClassNumber == CLASS_ENGINEER_IDX) {
				max_plasma = 30;
				PlasmaOn();

				TogglePowerArmor();
				SpawnHackPowerArmorUpdateThink();

				CBasePlayer::GiveAmmo(max_plasma, plasma_ammo_type, false);

			} else if (m_iClassNumber == CLASS_EXTERMINATOR_IDX) {
				max_plasma = GetAmmoDef()->MaxCarry(plasma_ammo_type);
				PlasmaOn();

				TogglePowerArmor();
				SpawnHackPowerArmorUpdateThink();
			}
		}
	}

	bugGlow = false;
}

void CHL2MP_Player::SpawnHackPowerArmorUpdateThink(void) {
	SetContextThink(&CHL2MP_Player::SendPowerArmorUpdate, gpGlobals->curtime + 0.1f, PLASMA_SHIELD_CTX);
}

void CHL2MP_Player::ShowHelpHint(void) {
	UTIL_HudHintText(ToBasePlayer(this), "#DK_Hint_Help");
}

void CHL2MP_Player::ShowDetailedHint(void) {
	int team = GetTeamNumber();
	const char *hint = NULL;

	if (team == TEAM_SPIDERS) {
		switch (m_iClassNumber) {
		case CLASS_BREEDER_IDX:
			hint = "#DK_Hint_ClassBreeder";
			break;
		case CLASS_HATCHY_IDX:
			hint = "#DK_Hint_ClassHatchy";
			break;
		case CLASS_DRONE_IDX:
			hint = "#DK_Hint_ClassDrone";
			break;
		case CLASS_KAMI_IDX:
			hint = "#DK_Hint_ClassKami";
			break;
		case CLASS_STINGER_IDX:
			hint = "#DK_Hint_ClassStinger";
			break;
		case CLASS_GUARDIAN_IDX:
			hint = "#DK_Hint_ClassGuardian";
			break;
		case CLASS_STALKER_IDX:
			hint = "#DK_Hint_ClassStalker";
			break;
		default:
			Warning("No Such Spider Class!\n");
		}
	} else if (team == TEAM_HUMANS) {
		switch (m_iClassNumber) {
		case CLASS_ENGINEER_IDX:
			hint = "#DK_Hint_ClassEngy";
			break;
		case CLASS_GRUNT_IDX:
			hint = "#DK_Hint_ClassGrunt";
			break;
		case CLASS_SHOCK_IDX:
			hint = "#DK_Hint_ClassShock";
			break;
		case CLASS_HEAVY_IDX:
			hint = "#DK_Hint_ClassHeavy";
			break;
		case CLASS_COMMANDO_IDX:
			hint = "#DK_Hint_ClassCommando";
			break;
		case CLASS_EXTERMINATOR_IDX:
			hint = "#DK_Hint_ClassExterm";
			break;
		case CLASS_MECH_IDX:
			hint = "#DK_Hint_ClassMech";
			break;
		default:
			Warning("No Such Human Class\n");
		}
	} else {
		Warning("Bad Team, Couldn't select Hint\n");
	}

	if (hint) {
		UTIL_HudHintText(ToBasePlayer(this), hint);
	}
}

//
// this function helps the client hl2mp object know if the
// player object is standing still.  If the object is a guardian
// it toggles the invisible skin when the player is stopped...
//
// The client also uses this information to determine whether or not
// the player should be rendered with a red glowing overlay texture
// commandos won't be rendered that way if they are stopped...
//
void CHL2MP_Player::MovementThink(void) {
	float spd;

	SetNextThink(gpGlobals->curtime + 0.1f);

	spd = VectorLength(GetAbsVelocity());
	if (spd < 0.1f) {
		if (stopped == false) {
			if (stoppedTicks >= STOP_MOTION_TICKS) {
				stopped = true;
			} else {
				stoppedTicks++;
			}
		}
	} else {

		SetContextThink(NULL, 0, GUARDIAN_ARMOR_CTX);

		if (stopped == true) {

			stopped = false;
			stoppedTicks = 0;
		}
	}

	//
	// determine guardian visibility state by stopped variable
	// and attackMotion
	//

	if (stopped && attackMotion == false) {

		if (m_nSkin == 0) {
			if (GetTeamNumber() == TEAM_SPIDERS && m_iClassNumber == CLASS_GUARDIAN_IDX) {
				m_nSkin = 1;
			}

			ResetGuardianArmorRecharge();
		}

	} else {

		if (m_nSkin == 1) {
			if (GetTeamNumber() == TEAM_SPIDERS && m_iClassNumber == CLASS_GUARDIAN_IDX) {
				m_nSkin = 0;
			}
		}

	}
}

bool CHL2MP_Player::IsStopped(void) {
	return stopped;
}

void CHL2MP_Player::Slash(CBaseEntity *other, bool force) {
	trace_t tr;
	Vector forward;
	Vector endpos;

	// first, is this player a hatchy or kami
	if (GetTeamNumber() == TEAM_SPIDERS) {
		if (m_iClassNumber == CLASS_HATCHY_IDX || m_iClassNumber == CLASS_KAMI_IDX) {
			if (other->GetTeamNumber() == TEAM_HUMANS) {
				if (m_flNextHitTime < gpGlobals->curtime) {
				// ignore force parameter:
				// sometimes hatchy touches another player and slashes extremely fast
				// if we don't respect the timer
				// if (force == true || m_flNextHitTime < gpGlobals->curtime) {

					CDisablePredictionFiltering foo;

					EmitSound(HATCHY_SLASH_SOUND);

					ClearMultiDamage();
					
					m_flNextHitTime =  gpGlobals->curtime + HATCHY_HIT_RATE;

					AngleVectors(GetLocalAngles(), &forward, NULL, NULL);

					UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

					endpos = tr.endpos;

					CTakeDamageInfo dmg(this, this, HATCHY_DAMAGE, DMG_SLASH|DMG_NEVERGIB);

					CalculateMeleeDamageForce(&dmg, forward, endpos);
					
					other->DispatchTraceAttack(dmg, forward, &tr);
					
					ApplyMultiDamage();
				}
			}
		}
	}
}

void CHL2MP_Player::StartTouch(CBaseEntity *other) {
	Slash(other, true);
	BaseClass::StartTouch(other);
}

void CHL2MP_Player::Touch(CBaseEntity *other) {
	Slash(other, false);

	if (other == GetGroundEntity()) {
		if (other->IsPlayer() &&
			GetTeamNumber() == TEAM_SPIDERS &&
			other->GetTeamNumber() == TEAM_HUMANS &&
			m_iClassNumber == CLASS_STALKER_IDX &&
			IsAlive()) {
				
				//
				// stalkers crush players they stand on
				//
				Vector crushDir = Vector(0,0,-1);
				trace_t tr;

				ClearMultiDamage();
				CTakeDamageInfo dmg(this, this, 1400, DMG_CRUSH);
				CalculateMeleeDamageForce(&dmg,crushDir, GetAbsOrigin(), 0.05f);
				other->DispatchTraceAttack(dmg, crushDir, &tr);
				ApplyMultiDamage();
		}
			
		return;
	}

	if (other->GetMoveType() != MOVETYPE_VPHYSICS || other->GetSolid() != SOLID_VPHYSICS || (other->GetSolidFlags() & FSOLID_TRIGGER))
		return;

	IPhysicsObject *pPhys = other->VPhysicsGetObject();
	if ( !pPhys || !pPhys->IsMoveable() )
		return;

	SetTouchedPhysics( true );
}

void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	
}

bool CHL2MP_Player::ValidatePlayerModel( const char *pModel )
{
	switch(GetTeamNumber()) {
	case TEAM_SPIDERS:
		if (m_iClassNumber < NUM_SPIDER_CLASSES) {
			if (!Q_stricmp(dk_spider_classes[m_iClassNumber].model, pModel))
				return true;
		}
		break;

	case TEAM_HUMANS:
		if (m_iClassNumber < NUM_HUMAN_CLASSES) {
			if (!Q_stricmp(dk_human_classes[m_iClassNumber].model, pModel))
				return true;
		}
		break;

	default:
		break;
	}

	return false;
}

void CHL2MP_Player::SetPlayerTeamModel( void )
{
	const char *mdl;

	mdl = "models/Combine_Soldier.mdl";
	SetModel(mdl);
	//SetupPlayerSoundsByModel(mdl);

	return;
}

void CHL2MP_Player::SetPlayerModel( void )
{
	const char *mdl;

	mdl = "models/Combine_Soldier.mdl";
	SetModel(mdl);
	//SetupPlayerSoundsByModel(mdl);
	return;
}

void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	if ( Q_stristr( pModelName, "models/human") ) {
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
	} else if ( Q_stristr(pModelName, "police" ) ) {
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_METROPOLICE;
	} else if ( Q_stristr(pModelName, "combine" ) ) {
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	} else {
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	}
}

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	if (pWeapon->HideWeapon()) {
		pWeapon->SetWeaponVisible(false);
	}

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
}

void CHL2MP_Player::InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity ) {
	// Cleanup any old vphysics stuff.
	VPhysicsDestroyObject();
	
	CPhysCollide *pModel = PhysCreateBbox( classVectors->m_vHullMin, classVectors->m_vHullMax );
	CPhysCollide *pCrouchModel = PhysCreateBbox( classVectors->m_vDuckHullMin, classVectors->m_vDuckHullMax );

	SetupVPhysicsShadow( vecAbsOrigin, vecAbsVelocity, pModel, "player_stand", pCrouchModel, "player_crouch" );
}

void CHL2MP_Player::SpitUnFreeze(void) {
	m_Local.m_bAllowAutoMovement = true;
	EnableControl(TRUE);
}

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING ) {
		SetCollisionBounds( classVectors->m_vCrouchTraceMin, classVectors->m_vCrouchTraceMax );
	} else {
		SetCollisionBounds( classVectors->m_vHullMin, classVectors->m_vHullMax );
	}

	m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
}

void CHL2MP_Player::PlayerDeathThink()
{
	if ( !IsObserver() ) {

		// if chose_class is false, don't use BaseClass's Think..
		if (chose_class == true) {
			BaseClass::PlayerDeathThink();

		} else {
			SetNextThink(gpGlobals->curtime + 0.1f);

			if (choosing_class == false && 
				gpGlobals->curtime > (m_flDeathTime + DEATH_ANIMATION_TIME))
			{
				choosing_class = true;
				ShowViewPortPanel(PANEL_CLASS, true, NULL);
			}
		}
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_flDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if ( pEntityTransmitBits && !pEntityTransmitBits->Get( pPlayer->entindex() ) )
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pPlayer->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	float maxDistance = 1.5 * pPlayer->MaxSpeed() * sv_maxunlag.GetFloat();

	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );
	
	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_SPIDERS )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

// Set the activity based on an event or current state
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;

	float speed;

	speed = GetAbsVelocity().Length2D();

	
	// bool bRunning = true;

	//Revisit!
/*	if ( ( m_nButtons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( speed > 1.0f && speed < hl2_normspeed.GetFloat() - 20.0f )
		{
			bRunning = false;
		}
	}*/

	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	else if ( playerAnim == PLAYER_RELOAD )
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_HL2MP_JUMP )	// Still jumping
		{
			idealActivity = GetActivity( );
		}
		/*
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		*/
		else
		{
			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if ( speed > 0 )
				{
					/*
					if ( bRunning == false )
					{
						idealActivity = ACT_WALK;
					}
					else
					*/
					{
						idealActivity = ACT_HL2MP_RUN;
					}
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		idealActivity = TranslateTeamActivity( idealActivity );
	}
	
	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );

		// FIXME: this seems a bit wacked
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

		return;
	}
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );

			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
	
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}


extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
/*	if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
	{
		char szReturnString[128];
		Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch teams again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );

		ClientPrint( this, HUD_PRINTTALK, szReturnString );
		return;
	}*/

	bool bKill = false;

	if ( HL2MPRules()->IsTeamplay() != true && iTeam != TEAM_SPECTATOR )
	{
		//don't let them try to join combine or rebels during deathmatch.
		iTeam = TEAM_UNASSIGNED;
	}

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
		{
			bKill = true;
		}
	}

	BaseClass::ChangeTeam( iTeam );

	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	SetPlayerTeamModel();

	if (iTeam == TEAM_SPECTATOR) {
		RemoveAllItems( true );

		State_Transition( STATE_OBSERVER_MODE );
	}

	if (bKill == true) {
		CommitSuicide(false, true);
	}
}

#define MAX_TEAM_DELTA_PLAYERS 1

bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	int spiders;
	int humans;

	if (!GetGlobalTeam( team )) {
		Warning("Bad team selection %d\n", team);
		return false;
	}

	if (team == GetTeamNumber() &&
		(team == TEAM_SPIDERS || team == TEAM_HUMANS) &&
		IsObserver() == false) {
		return true;
	}

	if ( team == TEAM_SPECTATOR ) {
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			ShowViewPortPanel(PANEL_TEAM, true, NULL);
			return false;
		}

		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;

	} else {
		//
		// 0 == TEAM_UNASSIGNED = AUTO_JOIN
		//
		
		spiders = g_SpiderTeam->GetNumPlayers();
		humans = g_HumanTeam->GetNumPlayers();

		if (team == 0) {
			if (spiders < humans) {
				team = TEAM_SPIDERS;
			} else {
				team = TEAM_HUMANS;
			}
		} else if (team == TEAM_SPIDERS || team == TEAM_HUMANS) {
			int t = GetTeamNumber();
			
			// pretend it might be a team change
			if (t == TEAM_SPIDERS) {
				spiders--;
			} else if (t == TEAM_HUMANS) {
				humans--;
			}

			if (dk_team_balance.GetBool() == true) {
				if (team == TEAM_SPIDERS) {
					if (spiders + 1 > humans + MAX_TEAM_DELTA_PLAYERS)
						return false;
				} else if (team == TEAM_HUMANS) {
					if (humans + 1 > spiders + MAX_TEAM_DELTA_PLAYERS)
						return false;
				}
			}

		}

		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	// Switch their actual team...
	ChangeTeam(team);

	return true;
}

void CHL2MP_Player::CheckThrowPosition(const Vector &vecEye, Vector &vecSrc) {
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS + 2,GRENADE_RADIUS + 2,GRENADE_RADIUS + 2), Vector(GRENADE_RADIUS + 2,GRENADE_RADIUS + 2,GRENADE_RADIUS + 2), 
		PhysicsSolidMaskForEntity(), this, GetCollisionGroup(), &tr );
	
	if (tr.DidHit()) {
		vecSrc = tr.endpos;
	}
}

//
// throw a grenade based on type
//

void CHL2MP_Player::ThrowGrenade(int type) {
	CBaseGrenade *pGrenade;
	Vector vecEye;
	Vector vForward;
	Vector vRight;
	Vector vecSrc;
	Vector vecThrow;
	int speed_factor;
	float gren_damage;
	float gren_radius;
	float gren_timer;
	bool sticky;

	int type_guardian;
	int type_spike;
	int type_smoke;
	int type_acid;
	int type_frag;

	
	vecEye = EyePosition();
	EyeVectors( &vForward, &vRight, NULL );
	vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition(vecEye, vecSrc);
	vForward[2] += 0.1f;
	GetVelocity( &vecThrow, NULL );


	type_guardian = GetAmmoDef()->Index(GRENADE_GUARD);
	type_spike = GetAmmoDef()->Index(GRENADE_SPIKE);
	type_smoke = GetAmmoDef()->Index(GRENADE_SMOKE);
	type_acid = GetAmmoDef()->Index(GRENADE_ACID);
	type_frag = GetAmmoDef()->Index(GRENADE_FRAG);

	// guardian/stalker both use same type
	if (type == type_guardian || type == type_spike) {
		if (m_iClassNumber == CLASS_GUARDIAN_IDX) {
			sticky = true;
			speed_factor = 500;
		} else {
			sticky = false;
			speed_factor = 900;
		}

		gren_damage = 1.0f;
		gren_radius = 250.0f;
		gren_timer = 1.2f;

		vecThrow += vForward * speed_factor;
		pGrenade = GuardianGren_Create( vecSrc, vec3_angle, vecThrow, this, gren_timer, sticky);

	} else if (type == type_smoke) {
		speed_factor = 600;
		gren_damage = 1.0f;
		gren_radius = 250.0f;
		gren_timer = 2.5f;
	
		vecThrow += vForward * speed_factor;
		pGrenade = SmkGren_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), this, gren_timer, false );

	} else if (type == type_acid) {
		speed_factor = 500;
		gren_damage = 15.0f;
		gren_radius = 250.0f;
		gren_timer = 1.2f;

		vecThrow += vForward * speed_factor;
		pGrenade = AcidGren_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), this, gren_timer, false );

	} else if (type == type_frag) {
		speed_factor = 600;
		gren_damage = 275.0f;
		gren_radius = 250.0f;
		gren_timer = 2.5f;
	
		vecThrow += vForward * speed_factor;
		pGrenade = Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), this, gren_timer, false );

	} else {
		Warning("Unknown grenade type!");
		return;
	}

	if (pGrenade) {
		if ( m_lifeState != LIFE_ALIVE ) {
			GetVelocity( &vecThrow, NULL );

			IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
			if (pPhysicsObject) {
				pPhysicsObject->SetVelocity( &vecThrow, NULL );
			}
		}
		
		pGrenade->SetDamage( gren_damage );
		pGrenade->SetDamageRadius( gren_radius );
	}

	// player "shoot" animation
	SetAnimation( PLAYER_ATTACK1 );
}



bool CHL2MP_Player::ClientCommand( const CCommand &args )
{
	if ( FStrEq( args[0], "spectate" ) ) {

		if (ShouldRunRateLimitedCommand(args)) {
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;

	} else if (FStrEq(args[0], "taunt")) {
		
		if (!IsObserver() && IsAlive()) {
			if (ShouldRunRateLimitedCommand(args)) {
				if (taunt_sound) {
					CDisablePredictionFiltering foo;
					EmitSound(taunt_sound);
				}
			}
		}

		return true;

	} else if ( FStrEq( args[0], "jointeam" ))  {
		
		if (args.ArgC() < 2) {
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if (ShouldRunRateLimitedCommand(args)) {
			int iTeam = atoi( args[1] );
			HandleCommand_JoinTeam( iTeam );
		}
		return true;

	} else if (FStrEq( args[0], "joingame" )) {
		return true;
	} else if (FStrEq(args[0], "switchclass")) {
		int classNum;

		if (IsDead()) {
			if (args.ArgC() < 2)
				return true;
		
			choosing_class = false;
			classNum = atoi(args.Arg(1));

			// don't actually set class properties here
			// do that upon Spawn()

			if (GetTeamNumber() == TEAM_SPIDERS) {
				if (classNum < NUM_SPIDER_CLASSES && classNum >= 0) {
					if (m_iPlayerPoints >= dk_spider_classes[classNum].cost) {
						if (chose_class == false) {
							chose_class = true;
							m_iClassNumber = classNum;
							SubtractPlayerPoints(dk_spider_classes[classNum].cost);
						}
					}
				}

				return true;

			} else if (GetTeamNumber() == TEAM_HUMANS) {
				if (classNum < NUM_HUMAN_CLASSES && classNum >= 0) {
					if (m_iPlayerPoints >= dk_human_classes[classNum].cost) {
						if (chose_class == false) {
							chose_class = true;
							m_iClassNumber = classNum;
							SubtractPlayerPoints(dk_human_classes[classNum].cost);
						}
					}
				}

				return true;
			}

			choosing_class = true;
			ShowViewPortPanel(PANEL_CLASS, true, NULL);
		}
	} else if (FStrEq(args[0], "toggle_pack_item")) {
		if (GetTeamNumber() != TEAM_HUMANS) {
			pack_item_idx = -1;
			return true;
		}

		TogglePackIdx();
		return true;
	} else if (FStrEq(args[0], "use_pack_item")) {
		//
		// try to use the current item, then toggle
		//
		if (GetTeamNumber() != TEAM_HUMANS) {
			return true;
		}

		UsePackItem();
		return true;
	} else if (FStrEq(args[0], "use_primary")) {
		if (pri_weapon) {
			Weapon_Switch(pri_weapon);
			return true;
		}
	} else if (FStrEq(args[0], "use_secondary")) {
		if (sec_weapon) {
			Weapon_Switch(sec_weapon);
			return true;
		}
	} else if (FStrEq(args[0], "use_grenade")) {
		if (gren_weapon) {
			//
			// off-hand grenades done here
			//
			
			if (m_flNextGrenadeTime < gpGlobals->curtime && HasAnyAmmoOfType(grenade_type)) {
				m_flNextGrenadeTime = gpGlobals->curtime + GRENADE_THROW_INTERVAL;
				ThrowGrenade(grenade_type);
				RemoveAmmo(1, grenade_type);
			}

			return true;
		}
	} else if (FStrEq(args[0], "use_special")) {
		if (spec_weapon) {
			Weapon_Switch(spec_weapon);
			return true;
		} else if (GetTeamNumber() == TEAM_SPIDERS &&
			m_iClassNumber == CLASS_GUARDIAN_IDX &&
			!IsDead()) {

				if (next_infestation < gpGlobals->curtime) {
					InfestCorpse();
				} else {
					char buf[64];
					Q_snprintf(buf, sizeof(buf), "%.0f seconds before next infestation!\n", next_infestation - gpGlobals->curtime);
					UTIL_SayText(buf, this);
				}

				return true;
		}
	} else if (FStrEq(args[0], "cancel")) {
		if (choosing_class == true) {
			choosing_class = false;
		}

		return true;
	} else if (FStrEq(args[0], "builditem")) {
		int itemNum;
		CBaseCombatWeapon *current;

		if (args.ArgC() < 2)
				return true;

		if (GetTeamNumber() == TEAM_HUMANS && m_iClassNumber == CLASS_ENGINEER_IDX) {
			CWeaponEngy *engy_gun;

			itemNum = atoi(args.Arg(1));

			if (itemNum < 0 || itemNum > NUM_HUMAN_ITEMS)
				return true;

			current = GetActiveWeapon();
			if (current == NULL)
				return true;

			engy_gun = dynamic_cast<CWeaponEngy *>(current);
			if (engy_gun == NULL)
				return true;

			engy_gun->MakeItem(itemNum);

			return true;

		} else if (GetTeamNumber() == TEAM_SPIDERS && m_iClassNumber == CLASS_BREEDER_IDX) {
			CWeaponBreeder *breeder_gun;

			itemNum = atoi(args.Arg(1));

			if (itemNum < 0 || itemNum > NUM_SPIDER_ITEMS)
				return true;

			current = GetActiveWeapon();
			if (current == NULL)
				return true;

			breeder_gun = dynamic_cast<CWeaponBreeder *>(current);
			if (breeder_gun == NULL)
				return true;

			breeder_gun->MakeItem(itemNum);

			return true;
		}
	} else if (FStrEq(args[0], "vguicancel")) {
		if (choosing_class == true)
			choosing_class = false;

		return true;
	} else if (FStrEq(args[0], "dk_show_hint")) {
		if (!IsObserver() && IsAlive()) {
			ShowDetailedHint();
			return true;
		}
	} else if (FStrEq(args[0], "dk_toggle_shield")) {
		if (!IsObserver() && IsAlive()) {
			TogglePowerArmor();
			return true;
		}
	}

	return BaseClass::ClientCommand( args );
}

#define HEALTH_PACK_AMT 50.0f

void CHL2MP_Player::UseHealthPack(void) {
	EmitSound("SuitRecharge.Start");
	TakeHealth(HEALTH_PACK_AMT, DMG_GENERIC);
}

void CHL2MP_Player::UseItem(int item) {
	if (item < 0)
		return;

	switch (item) {
	case PACK_ITEM_TYPE_HEALTH:
		UseHealthPack();
		break;
	}
}

void CHL2MP_Player::UsePackItem(void) {
	switch (pack_item_idx) {
	case 0:
		UseItem(pack_item_0);
		pack_item_0 = -1;
		break;
	case 1:
		UseItem(pack_item_1);
		pack_item_1 = -1;
		break;
	case 2:
		UseItem(pack_item_2);
		pack_item_2 = -1;
		break;
	}

	TogglePackIdx();
}

void CHL2MP_Player::TogglePackIdx(void) {
	switch (pack_item_idx) {
	case -1:
		if (pack_item_0 > 0) {
			pack_item_idx = 0;
		} else if (pack_item_1 > 0) {
			pack_item_idx = 1;
		} else if (pack_item_2 > 0) {
			pack_item_idx = 2;
		} else {
			pack_item_idx = -1;
		}
		
		break;
	case 0:
		if (pack_item_1 > 0) {
			pack_item_idx = 1;
		} else if (pack_item_2 > 0) {
			pack_item_idx = 2;
		} else if (pack_item_0 > 0) {
			pack_item_idx = 0;
		} else {
			pack_item_idx = -1;
		}

		break;

	case 1:
		if (pack_item_2 > 0) {
			pack_item_idx = 2;
		} else if (pack_item_0 > 0) {
			pack_item_idx = 0;
		} else if (pack_item_1 > 0) {
			pack_item_idx = 1;
		} else {
			pack_item_idx = -1;
		}

		break;

	case 2:
		if (pack_item_0 > 0) {
			pack_item_idx = 0;
		} else if (pack_item_1 > 0) {
			pack_item_idx = 1;
		} else if (pack_item_2 > 0) {
			pack_item_idx = 2;
		} else {
			pack_item_idx = -1;
		}

		break;

	default:
		pack_item_idx = -1;
	}

}

//void CHL2MP_Player::SetViewOffset(const Vector v) {
//	BaseClass::SetViewOffset(v);
//}

const Vector CHL2MP_Player::GetPlayerMins(void) const {
	if (classVectors) {
		if ( IsObserver() ) {
			return classVectors->m_vObsHullMin;
		} else {
			if ( GetFlags() & FL_DUCKING ) {
				return classVectors->m_vDuckHullMin;
			} else {
				return classVectors->m_vHullMin;
			}
		}
	} else {
		return vec3_origin;
	}
}

const Vector CHL2MP_Player::GetPlayerMaxs(void) const {
	if (classVectors) {
		if ( IsObserver() ) {
			return classVectors->m_vObsHullMax;
		} else {
			if ( GetFlags() & FL_DUCKING ) {
				return classVectors->m_vDuckHullMax;
			} else {
				return classVectors->m_vHullMax;
			}
		}
	} else {
		return vec3_origin;
	}
}

void CHL2MP_Player::SetVectors(HL2MPViewVectors *v) {
	classVectors = v;
}

void CHL2MP_Player::SetupPlayerSounds(int idx) {
	m_iPlayerSoundType = idx;
}

int CHL2MP_Player::GetMaxArmor(void) {
	return max_armor;
}

void CHL2MP_Player::SetPlayerClass(int c) {
	
	taunt_sound = NULL;

	switch (GetTeamNumber()) {
	case TEAM_SPIDERS:
		if (c < NUM_SPIDER_CLASSES) {
			m_iClassNumber = c;
			max_armor = dk_spider_classes[c].armor;
			taunt_sound = dk_spider_classes[c].taunt_sound;
			SetArmorValue(dk_spider_classes[c].armor);
			SetMaxSpeed(dk_spider_classes[c].max_speed);
			SetMaxHealth(dk_spider_classes[c].health);
			SetHealth(dk_spider_classes[c].health);
			SetModel(dk_spider_classes[c].model);
			SetupPlayerSounds(dk_spider_classes[c].snd_type_idx);
			SetDefaultFOV(dk_spider_classes[c].fov);
			SetVectors(dk_spider_classes[c].vectors);
			//m_Local.m_bDrawViewmodel = true;
			grenade_weapon_type = dk_spider_classes[c].grenade_weapon;
			
			if (Q_strcmp(dk_spider_classes[c].grenade_type, GRENADE_NULL)) {
				grenade_type = GetAmmoDef()->Index(dk_spider_classes[c].grenade_type);
			}

			if (c == CLASS_KAMI_IDX) {
				m_nSkin = 1;
			} else {
				m_nSkin = 0;
			}
		}

		break;

	case TEAM_HUMANS:
		m_nSkin = 0;

		if (c < NUM_HUMAN_CLASSES) {
			m_iClassNumber = c;
			max_armor = dk_human_classes[c].armor;
			taunt_sound = dk_human_classes[c].taunt_sound;
			SetArmorValue(dk_human_classes[c].armor);
			SetMaxSpeed(dk_human_classes[c].max_speed);
			SetMaxHealth(dk_human_classes[c].health);
			SetHealth(dk_human_classes[c].health);
			SetModel(dk_human_classes[c].model);
			SetupPlayerSounds(dk_human_classes[c].snd_type_idx);
			SetDefaultFOV(dk_human_classes[c].fov);
			SetVectors(dk_human_classes[c].vectors);
			//m_Local.m_bDrawViewmodel = true;
			grenade_weapon_type = dk_human_classes[c].grenade_weapon;

			if (Q_strcmp(dk_human_classes[c].grenade_type, GRENADE_NULL)) {
				grenade_type = GetAmmoDef()->Index(dk_human_classes[c].grenade_type);
			}

		}

		break;

	default:
		break;
	}

}

void CHL2MP_Player::GivePoints(void) {
	AddPlayerPoints(10);
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			if ( sv_cheats->GetBool() ) {
				RefilAmmo(false);
				GiveDefaultItems();
			}
			break;
		case 102:
			if ( sv_cheats->GetBool() ) {
				GivePoints();
			}
			break;
		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	if (GetTeamNumber() == TEAM_HUMANS) {
		return IsEffectActive( EF_DIMLIGHT );
	} else {
		return bugGlow;
	}
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{	
	if ( flashlight.GetInt() > 0 && IsAlive() ) {
		if (GetTeamNumber() == TEAM_HUMANS) {
			AddEffects( EF_DIMLIGHT );
			EmitSound( "HL2Player.FlashlightOn" );
		} else {
			bugGlow = true;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	bugGlow = false;
	RemoveEffects( EF_DIMLIGHT );
	
	if(IsAlive()) {
		if (GetTeamNumber() == TEAM_HUMANS) {
			EmitSound( "HL2Player.FlashlightOff" );
		}
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity ) {
	//CBaseCombatWeapon *pGrenade;

	////Drop a grenade if it's primed.
	//if ( GetActiveWeapon() ) {
	//	pGrenade = Weapon_OwnsThisType(grenade_weapon_type);
	//	if ( GetActiveWeapon() == pGrenade ) {
	//		if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) ) {
	//			// use proper drop function...
	//			if (Q_strcmp(grenade_weapon_type, WEAPON_FRAG_GREN)) {
	//				DropPrimedFragGrenade(this, pGrenade);
	//				return;
	//			} else if (Q_strcmp(grenade_weapon_type, WEAPON_SMOKE_GREN)) {
	//				DropPrimedSmkGrenGrenade(this, pGrenade);
	//				return;
	//			} else {
	//				Warning("Unable to drop specified grenade type without drop callback\n");
	//				return;
	//			}
	//		}
	//	}
	//}

	//BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}

void CHL2MP_Player::RemoveSpikeGrenades(void) {
	CBaseEntity *ent;

	ent = NULL;

	if (GetTeamNumber() == TEAM_SPIDERS && m_iClassNumber == CLASS_GUARDIAN_IDX || m_iClassNumber == CLASS_STALKER_IDX) {
		while ((ent = gEntList.FindEntityByClassname(ent, "grenade_guardian")) != NULL) {
			CGrenadeGuardian *g = dynamic_cast<CGrenadeGuardian *>(ent);
			if (g->GetThrower() == this) {
				UTIL_Remove(g);
			}
		}
	}
}

void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );
	SetThink(NULL);

	PlasmaOff();
	SetContextThink(NULL, 0, GUARDIAN_ARMOR_CTX);

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	CreateRagdollEntity();

	DetonateTripmines();

	RemoveSpikeGrenades();

	CBaseEntity *pAttacker = info.GetAttacker();
	CHL2MP_Player *player;

	if (pAttacker) {
		int iScoreToAdd = 1;

		if (pAttacker == this) {
			iScoreToAdd = -1;
		}

		player = ToHL2MPPlayer(pAttacker);
		if (player) {
			if (player->GetTeamNumber() != GetTeamNumber()) {
				player->AddPlayerPoints(1);
				GetGlobalTeam( pAttacker->GetTeamNumber() )->AddScore( iScoreToAdd );
			} else {
				player->SubtractPlayerPoints(1);
			}
		}

	}

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();
}

int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo ) {
	int dmgtype;
	CTakeDamageInfo newinfo = inputInfo;

	//
	// See if the damage was DMG_PARALYZE (drone spit)
	// and stop player movement for a fixed period of time
	//


	dmgtype = inputInfo.GetDamageType();

	if (dmgtype & DMG_PARALYZE && GetTeamNumber() == TEAM_HUMANS) {
		last_spit_hit = gpGlobals->curtime;
		m_Local.m_bAllowAutoMovement = false;
		EnableControl(FALSE);
		SetContextThink(&CHL2MP_Player::SpitUnFreeze, gpGlobals->curtime + DRONE_SPIT_TIME, DRONE_SPIT_CTX);
		return 0;
	}

	if (dmgtype == DMG_FALL) {
		if (GetTeamNumber() == TEAM_SPIDERS) {
			if (m_iClassNumber == CLASS_HATCHY_IDX ||
				m_iClassNumber == CLASS_KAMI_IDX ||
				m_iClassNumber == CLASS_DRONE_IDX) {
					return 0;
			} else {
				newinfo.ScaleDamage(0.25f);
			}
		} else {
			// fall harder if it's a human
			// particularly if it's a mech
			if (m_iClassNumber == CLASS_MECH_IDX) {
				newinfo.ScaleDamage(6.5f);
			} else {
				newinfo.ScaleDamage(3.0f);
			}
		}
	}

	if (dmgtype == DMG_ACID) {
		if (GetTeamNumber() == TEAM_HUMANS) {
			if (m_iClassNumber == CLASS_EXTERMINATOR_IDX || m_iClassNumber == CLASS_MECH_IDX) {
				return 0;
			}
		} else {
			return 0;
		}
	}

	//
	// having armor conveys a bonus
	//
	if (dmgtype & DMG_ALWAYSGIB) {
		if (ArmorValue() > 0) {
			newinfo.ScaleDamage(0.90f);
		}
	} else {
		if (ArmorValue() > 0 ) {
			newinfo.ScaleDamage(0.80f);
		}
	}

	if (dmgtype & DMG_PLASMA) {
		if (!(dmgtype & DMG_ALWAYSGIB)) {
			//
			// this is exterm plasma (doesn't have alwaysgib set)
			// so nerf it against stingers
			// and buff it against guardians
			//

			//
			// not having ALWAYSGIB set means that if the player
			// had armor, the XT's plasma was already weakened by 30%
			//

			if (GetTeamNumber() == TEAM_SPIDERS) {
				if (m_iClassNumber == CLASS_STINGER_IDX) {
					newinfo.ScaleDamage(0.5f);
				} else if (m_iClassNumber == CLASS_GUARDIAN_IDX) {
					newinfo.ScaleDamage(1.5f);
				}
			}
		}
	}

	//
	// engies and breeders take extra damage
	// to discourage brave thinking ;)
	//
	if ((GetTeamNumber() == TEAM_SPIDERS && m_iClassNumber == CLASS_BREEDER_IDX) ||
		(GetTeamNumber() == TEAM_HUMANS && m_iClassNumber == CLASS_ENGINEER_IDX)) {

			newinfo.ScaleDamage(1.1f);

	}

	FilterDamage(newinfo);

	if (dmgtype & DMG_BLAST) {
		return BaseClass::OnTakeDamage(newinfo);
	}

	// humans and spiders only take explosion damage
	// from teammates...

	if (inputInfo.GetAttacker()) {
		if (inputInfo.GetAttacker()->IsPlayer()) {
			if (inputInfo.GetAttacker()->edict() == edict()) {

				if (dropC4) {
					DropC4Pack();
					dropC4 = false;
				}

				return BaseClass::OnTakeDamage(newinfo);
			}

			CHL2MP_Player *p = ToHL2MPPlayer(inputInfo.GetAttacker());
			if (p && p->GetTeamNumber() == TEAM_SPIDERS &&
				(p->m_iClassNumber == CLASS_HATCHY_IDX || p->m_iClassNumber == CLASS_KAMI_IDX) &&
				dmgtype & DMG_SLASH) {

					if (GetTeamNumber() == TEAM_HUMANS &&
						(m_iClassNumber == CLASS_GRUNT_IDX || m_iClassNumber == CLASS_SHOCK_IDX)) {

							// hatchy/kami does extra damage against grunt/shock (light armor)
							newinfo.ScaleDamage(RandomFloat(1.2f, 1.7f));

					}
			}
		}

		if (inputInfo.GetAttacker()->GetTeamNumber() == GetTeamNumber()) {
			return 0;
		}
	}

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	gamestats->Event_PlayerDamage( this, newinfo );

	if (dropC4) {
		DropC4Pack();
		dropC4 = false;
	}

	return BaseClass::OnTakeDamage( newinfo );
}

void CHL2MP_Player::DropC4Pack(void) {
	int c4idx;
	
	c4idx = GetAmmoDef()->Index("grenade_c4");

	if (HasAnyAmmoOfType(c4idx)) {
		RemoveAmmo(1, GetAmmoDef()->Index("grenade_c4"));
		(void)C4_Create(GetAbsOrigin(), GetAbsAngles(), vec3_origin, this, C4_TIMER);
	}
}

Vector CHL2MP_Player::BodyTarget(const Vector &posSrc, bool noisy) {
	Vector res;

	res = GetAbsOrigin();
	
	if (classVectors) {
		res.z += classVectors->m_vHullMax[2] / 2;
	}

	return res;
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	char szStepSound[128];

	Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.Die", GetPlayerModelSoundPrefix() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, NULL ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

void CHL2MP_Player::NoClipSpawn(CBaseEntity *spawn) {
	CEggEntity *egg = NULL;

	if (GetTeamNumber() == TEAM_SPIDERS) {
		if ((egg = dynamic_cast<CEggEntity *>(spawn)) != NULL) {
			egg->EntNoClip(this);
		}
	}
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void ) {
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	const char *pSpawnpointName = "info_player_deathmatch";

	pLastSpawnPoint = g_pLastSpawn;
	pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	g_pLastSpawn = pSpot;
	m_flSlamProtectTime = gpGlobals->curtime + 0.5;

	return pSpot;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}

