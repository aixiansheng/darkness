#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_hatchy.h"
#include "physics.h"
#include "mathlib/vector.h"
#include "convar.h"
#include "class_info.h"
#include "basecombatweapon_shared.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "game.h"
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "IEffects.h"
	#include "Sprite.h"
	#include "SpriteTrail.h"
	#include "beam_shared.h"
	#include "explode.h"
 
	#include "vphysics/constraints.h"
	#include "physics_saverestore.h"
 
#endif
 
//#include "effect_dispatch_data.h"
 
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BOLT_MODEL			"models/crossbow_bolt.mdl"
 
#define BOLT_VELOCITY		3000
#define PLAYER_FLY_VELOCITY	650
#define PLAYER_SLOW_FLY		200
#define MIN_DISTANCE		30.0f

#ifndef CLIENT_DLL
 
LINK_ENTITY_TO_CLASS( grapple_hook, CGrapplingHook );
 
BEGIN_DATADESC( CGrapplingHook )
	DEFINE_THINKFUNC( FlyThink ),
	DEFINE_THINKFUNC( HookedThink ),
	DEFINE_FUNCTION( HookTouch ),
 
	DEFINE_FIELD( m_hPlayer, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),
 
END_DATADESC()

CGrapplingHook *CGrapplingHook::HookCreate(
	const Vector &vecOrigin,
	const QAngle &angAngles,
	CBaseEntity *pentOwner)
{
	CGrapplingHook *pHook;
	CWeaponGrapple *pOwner;
	CBasePlayer *pPlayer;

	pOwner = (CWeaponGrapple *)pentOwner;
	if (!pOwner)
		return NULL;

	pPlayer = ToBasePlayer(pOwner->GetOwner());
	if (!pPlayer)
		return NULL;

	pHook = (CGrapplingHook *)CreateEntityByName("grapple_hook");
	if (!pHook)
		return NULL;

	UTIL_SetOrigin(pHook, vecOrigin);
	pHook->SetAbsAngles(angAngles);
	pHook->Spawn();
 
	pHook->m_hOwner = pOwner;
	pHook->SetOwnerEntity(pPlayer);
	pHook->m_hPlayer = pPlayer;

	return pHook;
}

CGrapplingHook::~CGrapplingHook( void ) {
}

bool CGrapplingHook::CreateVPhysics( void ) {
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );
	return true;
}

unsigned int CGrapplingHook::PhysicsSolidMaskForEntity() const {
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

void CGrapplingHook::Spawn( void ) {
	Precache( );
 
	SetModel( BOLT_MODEL );
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector(1,1,1), Vector(1,1,1) );
	SetSolid( SOLID_BBOX );
 
	AddEffects( EF_NODRAW );
 
	UpdateWaterState();
 
	SetTouch( &CGrapplingHook::HookTouch );
	SetThink( &CGrapplingHook::FlyThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}
 
 
void CGrapplingHook::Precache( void ) {
	PrecacheModel(BOLT_MODEL);
}

void CGrapplingHook::HookTouch( CBaseEntity *pOther ) {
	trace_t	tr;
	Vector vForward;
	Vector origin;
	Vector rootOrigin;

	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;
 
	if (pOther != m_hOwner && pOther->m_takedamage != DAMAGE_NO) {

		m_hOwner->NotifyHookDied();
		SetTouch(NULL);
		SetThink(NULL);
		UTIL_Remove( this );

	} else {
		tr = BaseClass::GetTouchTrace();
 
		if (pOther->GetMoveType() == MOVETYPE_NONE && !(tr.surface.flags & SURF_SKY)) {

			SetMoveType( MOVETYPE_NONE );
			SetTouch(NULL);

			grapple_dir = GetAbsVelocity();
			VectorNormalize(grapple_dir);
			grapple_dir *= PLAYER_FLY_VELOCITY;

			//
			// make the player fly in the direction that the
			// grappling hook was moving
			//

			m_hPlayer->grappling = true;
			m_hPlayer->SetMoveType(MOVETYPE_FLY);
			m_hPlayer->SetAbsVelocity(grapple_dir);
			
			dist = (m_hPlayer->GetAbsOrigin() - GetAbsOrigin()).Length();

			SetThink(&CGrapplingHook::HookedThink);
			SetNextThink(gpGlobals->curtime + 0.1f);

		} else {
			
			//
			// hit something like a sky or another movable object
			// so don't bother trying to grapple onto it
			//
			
			m_hOwner->NotifyHookDied();
			SetTouch(NULL);
			SetThink(NULL);
			UTIL_Remove( this );
		}
	}
}
 
void CGrapplingHook::HookedThink( void ) {
	float cur_dist;
	float factor;
	Vector player_v;
	Vector hook_pos;
	Vector player_pos;
	Vector new_vel;

	player_v = m_hPlayer->GetAbsVelocity();
	player_pos = m_hPlayer->GetAbsOrigin();
	hook_pos = GetAbsOrigin();

	cur_dist = (player_pos - hook_pos).Length();

	// stop the player if we reached min distance to hook (unreliable)
	if (cur_dist <= MIN_DISTANCE) {
		SetThink(NULL);

		m_hPlayer->SetAbsVelocity(vec3_origin);
		dist = cur_dist;
		return;
	} 

	//
	// if the player was deflected, attempt to redirect...
	// ideally, this would mimic gamemovement's TryPlayerMove() to slide
	// along planes in directions that bring the player closer to the hook,
	// but for now, this will do...
	//
	if (grapple_dir != player_v) {
		
		//
		// check if we've approached MIN_DISTANCE and increase think frequency
		// so that a velocity change due to overshooting
		// the hook won't result in an oscillating velocity
		//

		if (cur_dist <= MIN_DISTANCE * 2.0) {
			SetNextThink(gpGlobals->curtime);
			factor = PLAYER_SLOW_FLY;
		} else {
			SetNextThink(gpGlobals->curtime + 0.1f);
			factor = PLAYER_FLY_VELOCITY;
		}

		new_vel = hook_pos - player_pos;
		VectorNormalize(new_vel);

		new_vel *= factor;
		grapple_dir = new_vel;

		m_hPlayer->SetAbsVelocity(new_vel);
		dist = cur_dist;
		return;
	}
	
	//
	// if we've overshot the hook's position (this happens)
	// then stop, because the player probably hasn't gone far
	//
	if (cur_dist > dist) {
		SetThink(NULL);

		m_hPlayer->SetAbsVelocity(vec3_origin);
		dist = cur_dist;
		return;
	}

	//Warning("NO PROBLEMO\n");
	SetNextThink(gpGlobals->curtime + 0.1f);
	dist = cur_dist;
}

void CGrapplingHook::FlyThink( void ) {
	QAngle angNewAngles;
 
	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );
	SetNextThink( gpGlobals->curtime + 0.1f );
}
 
#endif
 
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrapple, DT_WeaponGrapple )
 
#ifdef CLIENT_DLL
void RecvProxy_HookDied( const CRecvProxyData *pData, void *pStruct, void *pOut ) {
	CWeaponGrapple *pGrapple = ((CWeaponGrapple*)pStruct);
 
	RecvProxy_IntToEHandle( pData, pStruct, pOut );
 
	CBaseEntity *pNewHook = pGrapple->GetHook();
 
	if ( pNewHook == NULL ) {
		if ( pGrapple->GetOwner() && pGrapple->GetOwner()->GetActiveWeapon() == pGrapple ) {
			pGrapple->NotifyHookDied();
		}
	}
}
#endif
 
BEGIN_NETWORK_TABLE( CWeaponGrapple, DT_WeaponGrapple )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bMustReload ) ),
	RecvPropEHandle( RECVINFO( m_hHook ), RecvProxy_HookDied ),
#else
	SendPropBool( SENDINFO( m_bMustReload ) ),
	SendPropEHandle( SENDINFO( m_hHook ) ),
#endif
END_NETWORK_TABLE()
 
#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponGrapple )
	DEFINE_PRED_FIELD( m_bMustReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif
 
LINK_ENTITY_TO_CLASS( weapon_hatchy, CWeaponGrapple );
 
PRECACHE_WEAPON_REGISTER( weapon_hatchy );
 
//#ifndef CLIENT_DLL
 
acttable_t	CWeaponGrapple::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,						ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_WALK_CROUCH,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,		ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};
 
IMPLEMENT_ACTTABLE(CWeaponGrapple);
 
//#endif

CWeaponGrapple::CWeaponGrapple( void ) {
	m_bReloadsSingly	= true;
	m_bFiresUnderwater	= true;
	m_bMustReload		= false;
	exploder			= false;
	invisible			= true;
}

void CWeaponGrapple::Precache( void ) {

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "grapple_hook" );
#endif

	BaseClass::Precache();
}

void CWeaponGrapple::PrimaryAttack( void ) {
	if (m_hHook)
		return;
 
	FireHook();
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration( ACT_VM_PRIMARYATTACK ) );
}

//
// borrowed from weapon_rpg.cpp
//
void CWeaponGrapple::SecondaryAttack( void ) {
#ifndef CLIENT_DLL
	CHL2MP_Player *p;

	p = ToHL2MPPlayer(GetOwnerEntity());
	if (!p)
		return;
	
	//
	// exploding kami's shouldn't leave bodies behind
	//
	p->ForceNoRagdoll(true);

	//
	// must ignore the owner to avoid crashes
	//
	ExplosionCreate
	(
		GetAbsOrigin(),
		GetAbsAngles(),
		GetOwnerEntity(),
		KAMI_DAMAGE,
		KAMI_RADIUS,
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE,
		0.0f,
		this
	);

	//
	// manually kill the player since explosion ignored him
	//
	if (p->IsAlive()) {
		p->CommitSuicide(true, true);
	}

#endif
}

void CWeaponGrapple::Drop(const Vector &vecVelocity) {
}

bool CWeaponGrapple::Reload( void ) {
	if (m_bMustReload && ( m_flNextPrimaryAttack <= gpGlobals->curtime)) {
		SendWeaponAnim(ACT_VM_RELOAD);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
		m_bMustReload = false;
	}
 
	return true;
}

void CWeaponGrapple::ItemPostFrame( void ) {
	CBasePlayer *pOwner;

	pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
 
	if ((pOwner->m_nButtons & IN_ATTACK2) && exploder == true) {
		SecondaryAttack();
		return;
	}

	if (pOwner->m_nButtons & IN_ATTACK) {
		if (m_flNextPrimaryAttack < gpGlobals->curtime) {
			PrimaryAttack();
		}
	} else if (m_bMustReload) {
		Reload();
	}
 
#ifndef CLIENT_DLL
	if (m_hHook) {
		if (!(pOwner->m_nButtons & IN_ATTACK)) {
			// player unhooked...
			m_hHook->SetTouch( NULL );
			m_hHook->SetThink( NULL );
 
			UTIL_Remove( m_hHook );
			m_hHook = NULL;
 
			NotifyHookDied();
			pOwner->SetMoveType(MOVETYPE_WALK);
			pOwner->grappling = false;

			m_bMustReload = true;
		}
	}
#endif

}
 
void CWeaponGrapple::FireHook( void ) {
	CBasePlayer *pOwner;

	if (m_bMustReload)
		return;
 
	pOwner = ToBasePlayer( GetOwner() );
 
	if (!pOwner)
		return;
 
#ifndef CLIENT_DLL

	Vector vecAiming;	
	Vector vecSrc;
	QAngle angAiming;
	CGrapplingHook *pHook;

	vecAiming	= pOwner->GetAutoaimVector( 0 );	
	vecSrc		= pOwner->Weapon_ShootPosition();
	VectorAngles(vecAiming, angAiming);
 
	pHook = CGrapplingHook::HookCreate(vecSrc, angAiming, this);
	if (!pHook)
		return;

	pHook->SetAbsVelocity( vecAiming * BOLT_VELOCITY );
	m_hHook = pHook;

#endif
 
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
 
	m_flNextPrimaryAttack = m_flNextSecondaryAttack	= gpGlobals->curtime + 0.1f;
}

bool CWeaponGrapple::Deploy( void ) {
	bool ret;

	invisible = true;

#ifndef CLIENT_DLL
	CHL2MP_Player *p;
	p =  ToHL2MPPlayer(GetOwner());

	if (p && p->GetTeamNumber() == TEAM_SPIDERS && p->m_iClassNumber == CLASS_KAMI_IDX) {
		exploder = true;
	}
#endif

	if ( m_bMustReload )
		return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_CROSSBOW_DRAW_UNLOADED, (char*)GetAnimPrefix() );
 
	ret = BaseClass::Deploy();
	
	return ret;
}

bool CWeaponGrapple::SendWeaponAnim( int iActivity ) {
	int newActivity = iActivity;
	return BaseClass::SendWeaponAnim( newActivity );
}

bool CWeaponGrapple::HasAnyAmmo( void ) {
	if ( m_hHook != NULL )
		return true;
 
	return BaseClass::HasAnyAmmo();
}

void CWeaponGrapple::NotifyHookDied( void ) {
	m_hHook = NULL;
}