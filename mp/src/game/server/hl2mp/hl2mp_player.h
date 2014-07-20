//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef HL2MP_PLAYER_H
#define HL2MP_PLAYER_H
#pragma once

class CHL2MP_Player;

#include "basemultiplayerplayer.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "simtimer.h"
#include "soundenvelope.h"
#include "hl2mp_player_shared.h"
#include "hl2mp_gamerules.h"
#include "utldict.h"
#include "physics_prop_ragdoll.h"

#define MAX_PLAYER_POINTS 10
#define HEAL_SOUND_INTERVAL 5.0f
#define STOP_MOTION_TICKS 3

class HL2MPViewVectors;

//=============================================================================
// >> HL2MP_Player
//=============================================================================
class CHL2MPPlayerStateInfo
{
public:
	HL2MPPlayerState m_iPlayerState;
	const char *m_pStateName;

	void (CHL2MP_Player::*pfnEnterState)();	// Init and deinit the state.
	void (CHL2MP_Player::*pfnLeaveState)();

	void (CHL2MP_Player::*pfnPreThink)();	// Do a PreThink() in this state.
};

class CHL2MP_Player : public CHL2_Player
{
public:
	DECLARE_CLASS( CHL2MP_Player, CHL2_Player );

	CHL2MP_Player();
	~CHL2MP_Player( void );
	
	static CHL2MP_Player *CreatePlayer( const char *className, edict_t *ed )
	{
		CHL2MP_Player::s_PlayerEdict = ed;
		return (CHL2MP_Player*)CreateEntityByName( className );
	}

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

#ifdef DEBUG
	void BotTeamClass(int team, int classNum);
#endif

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void PostThink( void );
	virtual void PreThink( void );
	virtual void PlayerDeathThink( void );
	virtual void SetAnimation( PLAYER_ANIM playerAnim );
	virtual bool HandleCommand_JoinTeam( int team );
	virtual bool ClientCommand( const CCommand &args );
	virtual void CreateViewModel( int viewmodelindex = 0 );
	virtual bool BecomeRagdollOnClient( const Vector &force );
	
	//virtual void Event_Dying( const CTakeDamageInfo &info );
	virtual bool ShouldGib( const CTakeDamageInfo &info );

	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual int OnTakeDamage( const CTakeDamageInfo &inputInfo );
	
	virtual bool WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const;
	virtual void FireBullets ( const FireBulletsInfo_t &info );
	virtual bool Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0);
	virtual bool BumpWeapon( CBaseCombatWeapon *pWeapon );
	virtual void ChangeTeam( int iTeam );
	virtual void PickupObject ( CBaseEntity *pObject, bool bLimitMassAndSize );
	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual void Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = NULL, const Vector *pVelocity = NULL );
	virtual void UpdateOnRemove( void );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual CBaseEntity* EntSelectSpawnPoint( void );

	virtual bool BecomeRagdoll(const CTakeDamageInfo &info, const Vector &forceVector);
	void MovementThink(void);
	bool IsStopped(void);
	void DropC4OnHit(bool value);
	void TogglePowerArmor(void);

	void SpeedPenalty(float duration);
	void SetNormalSpeed(void);

	void NoAttackMotion(void);
	void AttackMotion(void);
	CNetworkVar(bool, attackMotion);

	void ShowHelpHint(void);
	void ShowDetailedHint(void);

	virtual Vector BodyTarget(const Vector &posSrc, bool noisy);
		
	int FlashlightIsOn( void );
	void FlashlightTurnOn( void );
	void FlashlightTurnOff( void );
	void PrecacheFootStepSounds( void );
	bool ValidatePlayerModel( const char *pModel );

	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles.Get(); }

	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL );

	void CheatImpulseCommands( int iImpulse );
	void CreateRagdollEntity( void );
	void GiveAllItems( void );
	void GiveDefaultItems( void );
	void RefilAmmo(bool small);
	bool EntRefil(float delay);

	void NoteWeaponFired( void );

	void ResetAnimation( void );
	void SetPlayerModel( void );
	void SetPlayerTeamModel( void );
	Activity TranslateTeamActivity( Activity ActToTranslate );
	
	float GetNextModelChangeTime( void ) { return m_flNextModelChangeTime; }
	float GetNextTeamChangeTime( void ) { return m_flNextTeamChangeTime; }
	void  PickDefaultSpawnTeam( void );
	void  SetupPlayerSoundsByModel( const char *pModelName );
	const char *GetPlayerModelSoundPrefix( void );
	int	  GetPlayerModelType( void ) { return m_iPlayerSoundType;	}
	
	void DetonateTripmines(void);
	void RemoveSpikeGrenades(void);

	void Reset();

	bool IsReady();
	void SetReady( bool bReady );

	void CheckChatText( char *p, int bufsize );

	void State_Transition( HL2MPPlayerState newState );
	void State_Enter( HL2MPPlayerState newState );
	void State_Leave();
	void State_PreThink();
	CHL2MPPlayerStateInfo *State_LookupInfo( HL2MPPlayerState state );

	void State_Enter_ACTIVE();
	void State_PreThink_ACTIVE();
	void State_Enter_OBSERVER_MODE();
	void State_PreThink_OBSERVER_MODE();


	virtual bool StartObserverMode( int mode );
	virtual void StopObserverMode( void );


	Vector m_vecTotalBulletForce;	//Accumulator for bullet force in a single frame

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle

	virtual bool	CanHearAndReadChatFrom( CBasePlayer *pPlayer );

		
	int GetPlayerPoints(void) { return m_iPlayerPoints; }
	void AddPlayerPoints(int i);
	void SubtractPlayerPoints(int i);
	void SetPlayerPoints(int i) { m_iPlayerPoints = i; }
	void SetPlayerClass(int c);
	void SetMaxSpeed(float speed);
	//void SetViewOffset(const Vector v);
	const Vector GetPlayerMins(void) const;
	const Vector GetPlayerMaxs(void) const;
	void SetVectors(HL2MPViewVectors *vecs);
	void InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity );
	HL2MPViewVectors *classVectors;
	void Touch( CBaseEntity *other );
	void StartTouch( CBaseEntity *other );
	void Slash(CBaseEntity *other, bool force);

	void SetupPlayerSounds(int idx);

	bool ShowSpawnHint(void);
	void DisableSpawnHint(void);
	void DropC4Pack(void);
	void ParalyzePlayer(void);

	virtual bool HasHumanGibs(void);
	virtual bool HasAlienGibs(void);

	CNetworkVar (int, m_iClassNumber);
	
	float next_heal_sound;

	void NoClipSpawn(CBaseEntity *spawn);
	
	float DmgTypeFactor(int dmgtype);

	void PlasmaOn(void);
	void PlasmaOff(void);
	bool PlasmaReady(void);
	void RechargeThink(void);
	void FilterDamageArmor(CTakeDamageInfo &info);
	int PlasmaCharge(void);
	void PlasmaShot(void);
	void RailgunShot(void);
	void BurnPlasma(int amt);
	void DisablePlasma(bool dmg);
	void RegularRecharge(void);
	bool JetOn(void);
	int NumPlasmaShotsLeft(void);

	void StartJetPack(void);
	void StopJetPack(void);
	
	void JetOnThink(void);
	void JetIgniteThink(void);

	virtual void CommitSuicide(bool bExplode = false, bool bForce = false);

	void GuardianArmorRechargeThink(void);
	void ResetGuardianArmorRecharge(void);

	virtual void PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper);

	void UseHealthPack(void);

	int GetMaxArmor(void);

	void SwallowC4Sound(void);

	void ThrowGrenade(int type);
	void CheckThrowPosition(const Vector &vecEye, Vector &vecSrc);

	void SendPowerArmorUpdate(void);
	void SpawnHackPowerArmorUpdateThink(void);

	void Detected(void);
	void UnDetectThink(void);

	void ForceNoRagdoll(bool force);
	bool PlasmaArmorEnabled(void);

	CNetworkVar(bool, stopped);
	CNetworkVar(int, spawnNumber);

private:

	CNetworkQAngle( m_angEyeAngles );
	CPlayerAnimState   m_PlayerAnimState;

	int m_iLastWeaponFireUsercmd;
	int m_iModelType;
	
	CNetworkVar( int, m_iPlayerPoints );
	CNetworkVar( int, m_iSpawnInterpCounter );
	CNetworkVar( int, m_iPlayerSoundType );

	float m_flNextModelChangeTime;
	float m_flNextTeamChangeTime;

	float m_flSlamProtectTime;	

	HL2MPPlayerState m_iPlayerState;
	CHL2MPPlayerStateInfo *m_pCurStateInfo;

	bool ShouldRunRateLimitedCommand( const CCommand &args );

	void SpitUnFreeze(void);

	void GivePoints(void);

	// This lets us rate limit the commands the players can execute so they don't overflow things like reliable buffers.
	CUtlDict<float,int>	m_RateLimitLastCommandTimes;

    bool m_bEnterObserver;
	bool m_bReady;

	bool forceNoRagdoll;

	// class and class-selection related stuff
	bool chose_class;
	bool choosing_class;

	float m_flNextHitTime;
	float last_spit_hit;

	int num_flares;
	float m_flNextFlareTime;
	float m_flNextAmmoDenySound;
	float m_flNextHealthPack;
	float m_flNextRefilTime;
	float m_flNextShieldToggle;
	float m_flNextShieldToggleTry;

	float m_flNextGrenadeTime;
	CNetworkVar(int, grenade_type);

	int num_hints;
	int max_armor;
	int stoppedTicks;
	bool dropC4;

	const char *grenade_weapon_type;
	const char *taunt_sound;

	int plasma_ammo_type;
	int max_plasma;
	int recharge_mul;
	bool plasma_recovering;
	float recharge_start;
	float jet_start;

	CNetworkVar(bool, has_health_pack);
	CNetworkVar(bool, jetpack_on);
	CNetworkVar(bool, plasma_ready);
	CNetworkVar(bool, powerArmorEnabled);
	CNetworkVar(bool, bugGlow);

	CBaseCombatWeapon *pri_weapon;
	CBaseCombatWeapon *sec_weapon;
	CBaseCombatWeapon *gren_weapon;
	CBaseCombatWeapon *spec_weapon;

	float next_infestation;

	float m_flNextEntRefil;

	void InfestCorpse(void);

	bool showSpawnHint;

	int hatchy_ammo_type;

	float deathHealth;
};

inline CHL2MP_Player *ToHL2MPPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CHL2MP_Player*>( pEntity );
}

#endif //HL2MP_PLAYER_H
