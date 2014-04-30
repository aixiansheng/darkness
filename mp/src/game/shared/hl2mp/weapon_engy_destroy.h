#ifndef HL2MP_WEAPON_ENGY_DESTROY_H
#define HL2MP_WEAPON_ENGY_DESTROY_H
#pragma once

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#ifdef CLIENT_DLL
	#include "fx_interpvalue.h"
	#include "clienteffectprecachesystem.h"
	#include "vcollide_parse.h"
	#include "engine/ivdebugoverlay.h"
	#include "iviewrender_beams.h"
	#include "beamdraw.h"
	#include "c_te_effect_dispatch.h"
	#include "model_types.h"
#else
	#include "soundent.h"
	#include "ndebugoverlay.h"
	#include "ai_basenpc.h"
	#include "player_pickup.h"
	#include "physics_prop_ragdoll.h"
	#include "globalstate.h"
	#include "props.h"
	#include "te_effect_dispatch.h"
	#include "util.h"
#endif

#include "gamerules.h"
#include "soundenvelope.h"
#include "engine/IEngineSound.h"
#include "physics.h"
#include "in_buttons.h"
#include "IEffects.h"
#include "shake.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "physics_saverestore.h"
#include "movevars_shared.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "vphysics/friction.h"
#include "debugoverlay_shared.h"


enum destroy_gun_state {
	STATE_OFF,
	STATE_CHARGING,
	STATE_CHARGED,
	STATE_DRAINING
};

typedef enum destroy_gun_state destroy_gun_state_t;

#ifdef CLIENT_DLL
#define CWeaponEngyDestroy C_WeaponEngyDestroy
#include "c_team.h"
#endif

//#define PHYSCANNON_BEAM_SPRITE "sprites/orangelight1.vmt"
//#define PHYSCANNON_BEAM_SPRITE_NOZ "sprites/orangelight1_noz.vmt"
//#define PHYSCANNON_GLOW_SPRITE "sprites/glow04_noz"
//#define PHYSCANNON_ENDCAP_SPRITE "sprites/orangeflare1"
//#define PHYSCANNON_CENTER_GLOW "sprites/orangecore1"
//#define PHYSCANNON_BLAST_SPRITE "sprites/orangecore2"

#define PHYSCANNON_BEAM_SPRITE "sprites/lgtning_noz.vmt"
#define PHYSCANNON_BEAM_SPRITE_NOZ "sprites/lgtning_noz.vmt"
#define PHYSCANNON_GLOW_SPRITE "sprites/blueflare1_noz.vmt"
#define PHYSCANNON_ENDCAP_SPRITE "sprites/blueflare1_noz.vmt"
#define PHYSCANNON_CENTER_GLOW "effects/fluttercore.vmt"
#define PHYSCANNON_BLAST_SPRITE "effects/fluttercore.vmt"


#ifdef CLIENT_DLL

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//  CPhysCannonEffect class
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class CPhysCannonEffect
{
public:
	CPhysCannonEffect( void ) : m_vecColor( 255, 255, 255 ), m_bVisible( true ), m_nAttachment( -1 ) {};

	void SetAttachment( int attachment ) { m_nAttachment = attachment; }
	int	GetAttachment( void ) const { return m_nAttachment; }

	void SetVisible( bool visible = true ) { m_bVisible = visible; }
	int IsVisible( void ) const { return m_bVisible; }

	void SetColor( const Vector &color ) { m_vecColor = color; }
	const Vector &GetColor( void ) const { return m_vecColor; }

	bool SetMaterial(  const char *materialName )
	{
		m_hMaterial.Init( materialName, TEXTURE_GROUP_CLIENT_EFFECTS );
		return ( m_hMaterial != NULL );
	}

	CMaterialReference &GetMaterial( void ) { return m_hMaterial; }

	CInterpolatedValue &GetAlpha( void ) { return m_Alpha; }
	CInterpolatedValue &GetScale( void ) { return m_Scale; }

private:
	CInterpolatedValue	m_Alpha;
	CInterpolatedValue	m_Scale;

	Vector				m_vecColor;
	bool				m_bVisible;
	int					m_nAttachment;
	CMaterialReference	m_hMaterial;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//  CPhysCannonEffectBeam class
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class CPhysCannonEffectBeam
{
public:
	CPhysCannonEffectBeam( void ) : m_pBeam( NULL ) {};

	~CPhysCannonEffectBeam( void )
	{
		Release();
	}

	void Release( void )
	{
		if ( m_pBeam != NULL )
		{
			m_pBeam->flags = 0;
			m_pBeam->die = gpGlobals->curtime - 1;
			
			m_pBeam = NULL;
		}
	}

	void Init( int startAttachment, int endAttachment, CBaseEntity *pEntity, bool firstPerson )
	{
		if ( m_pBeam != NULL )
			return;

		BeamInfo_t beamInfo;

		beamInfo.m_pStartEnt = pEntity;
		beamInfo.m_nStartAttachment = startAttachment;
		beamInfo.m_pEndEnt = pEntity;
		beamInfo.m_nEndAttachment = endAttachment;
		beamInfo.m_nType = TE_BEAMPOINTS;
		beamInfo.m_vecStart = vec3_origin;
		beamInfo.m_vecEnd = vec3_origin;
		
		beamInfo.m_pszModelName = ( firstPerson ) ? PHYSCANNON_BEAM_SPRITE_NOZ : PHYSCANNON_BEAM_SPRITE;
		
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.0f;
		
		if ( firstPerson )
		{
			beamInfo.m_flWidth = 0.0f;
			beamInfo.m_flEndWidth = 4.0f;
		}
		else
		{
			beamInfo.m_flWidth = 0.5f;
			beamInfo.m_flEndWidth = 2.0f;
		}

		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 16;
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 150.0f;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 30.0;
		beamInfo.m_flRed = 255.0;
		beamInfo.m_flGreen = 255.0;
		beamInfo.m_flBlue = 255.0;
		beamInfo.m_nSegments = 8;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = FBEAM_FOREVER;
	
		m_pBeam = beams->CreateBeamEntPoint( beamInfo );
	}

	void SetVisible( bool state = true )
	{
		if ( m_pBeam == NULL )
			return;

		m_pBeam->brightness = ( state ) ? 255.0f : 0.0f;
	}

private:
	Beam_t	*m_pBeam;
};

#endif


class CWeaponEngyDestroy : public CBaseHL2MPBludgeonWeapon {
public:
	DECLARE_CLASS( CWeaponEngyDestroy, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponEngyDestroy();

	float		GetRange( void );
	float		GetFireRate( void );

	float		GetDamageForActivity( Activity hitActivity );
	void		PrimaryAttack(void);

	void		Drop( const Vector &vecVelocity );
	bool		Deploy( void );

	virtual void ItemPostFrame( void );
	virtual void Precache(void);
	virtual void UpdateOnRemove(void);
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	CWeaponEngyDestroy( const CWeaponEngyDestroy & );

	void ChargeThink(void);
	void DrainThink(void);
	void DamageOwner(void);

	void OpenElements(void);
	void CloseElements(void);

	void ShockEntityEffect(CBaseEntity *pEntity, const Vector &forward, trace_t &tr);

	void ChargingBeep(int percent);
	float ChargeBeepQuickening(int percent);

	/////////////////////////////////////////
	// effect methods
	/////////////////////////////////////////
	void DoEffect(int effectType, Vector *pos = NULL);

	void StartEffects(void);	// Initialize all sprites and beams
	void StopEffects(bool stopSound = true);	// Hide all effects temporarily
	void DestroyEffects(void);

	// Physgun effects
	void DoEffectClosed( void );
	void DoEffectReady( void );
	void DoEffectHolding( void );
	void DoEffectLaunch( Vector *pos );
	void DoEffectNone( void );
	void DoEffectIdle( void );

	// Trace length
	float TraceLength();

	// Sprite scale factor 
	float SpriteScaleFactor();

	void DryFire( void );
	void PrimaryFireEffect( void );

	CSoundPatch *GetMotorSound(void);
	void StopLoopingSounds(void);

	CSoundPatch *m_sndMotor;

	CNetworkVar(bool, clawsOpen);
	CNetworkVar(int, m_EffectState);

#ifndef CLIENT_DLL

	virtual void Spawn(void);
	void ItemStatusUpdate(CBasePlayer *player, int health, int armor);

	DECLARE_DATADESC();

#else

	enum EffectType_t
	{
		PHYSCANNON_CORE = 0,
		
		PHYSCANNON_BLAST,

		PHYSCANNON_GLOW1,	// Must be in order!
		PHYSCANNON_GLOW2,
		PHYSCANNON_GLOW3,
		PHYSCANNON_GLOW4,
		PHYSCANNON_GLOW5,
		PHYSCANNON_GLOW6,

		PHYSCANNON_ENDCAP1,	// Must be in order!
		PHYSCANNON_ENDCAP2,
		PHYSCANNON_ENDCAP3,	// Only used in third-person!

		NUM_PHYSCANNON_PARAMETERS	// Must be last!
	};

	#define	NUM_GLOW_SPRITES ((CWeaponEngyDestroy::PHYSCANNON_GLOW6-CWeaponEngyDestroy::PHYSCANNON_GLOW1)+1)
	#define NUM_ENDCAP_SPRITES ((CWeaponEngyDestroy::PHYSCANNON_ENDCAP3-CWeaponEngyDestroy::PHYSCANNON_ENDCAP1)+1)
	#define	NUM_PHYSCANNON_BEAMS 3

	virtual int DrawModel(int flags);
	virtual void ViewModelDrawn(C_BaseViewModel *pBaseViewModel);
	virtual bool IsTransparent(void);

	void OnDataChanged(DataUpdateType_t type);
	void UpdateElementPosition(void);
	void ClientThink(void);
	void NotifyShouldTransmit(ShouldTransmitState_t state);
	void DrawEffects(void);
	void GetEffectParameters(EffectType_t effectID, color32 &color, float &scale, IMaterial **pMaterial, Vector &vecAttachment);
	void DrawEffectSprite(EffectType_t effectID);
	inline bool	IsEffectVisible(EffectType_t effectID);

	RenderGroup_t GetRenderGroup( void ) {
		return RENDER_GROUP_TWOPASS;
	}

	//
	// describes the  interpolated position of the claw open/close
	// position as a float or something
	//
	CInterpolatedValue m_ElementParameter;
	CPhysCannonEffect m_Parameters[NUM_PHYSCANNON_PARAMETERS];	// Interpolated parameters for the effects
	CPhysCannonEffectBeam m_Beams[NUM_PHYSCANNON_BEAMS];

	bool oldClawsOpen;
	int m_nOldEffectState;

#endif

private:
	float m_flNextItemStatus;
	
	CNetworkVar(float, nextWarnBeep);
	CNetworkVar(int, warnBeepsLeft);
	CNetworkVar(destroy_gun_state_t, gun_state);
};


#endif // HL2MP_WEAPON_ENGY_DESTROY_H

