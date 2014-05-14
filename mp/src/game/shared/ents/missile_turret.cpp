#include "cbase.h"
#include "missile_turret.h"
#include "mg_turret.h"
#include "item_info.h"
#include "class_info.h"

#define MSL_PITCH_RATE	100
#define MSL_PITCH_RANGE	50
#define MSL_PITCH_TOL	10

#define MSL_YAW_RATE	MSL_PITCH_RATE
#define MSL_YAW_RANGE	180
#define MSL_YAW_TOL		MSL_PITCH_TOL

#define MSL_NO_TARGET_INTERVAL		0.1f
#define MSL_HAVE_TARGET_INTERVAL	0.05f


LINK_ENTITY_TO_CLASS(ent_seeker_msl, CSeekerMissile);
IMPLEMENT_NETWORKCLASS_ALIASED( SeekerMissile, DT_SeekerMissile );

BEGIN_NETWORK_TABLE(CSeekerMissile, DT_SeekerMissile)
END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS (ent_msl_turret, CMSLTurretEntity );
IMPLEMENT_NETWORKCLASS_ALIASED( MSLTurretEntity, DT_MSLTurretEntity );

BEGIN_NETWORK_TABLE( CMSLTurretEntity, DT_MSLTurretEntity )
END_NETWORK_TABLE()

CMSLTurretEntity::CMSLTurretEntity() : CHumanMateriel(&dk_items[ITEM_MSL_TURRET_IDX]) {}
CMSLTurretEntity::~CMSLTurretEntity() {}


LINK_ENTITY_TO_CLASS (msl_turret_head, CMSLTurretHead );
IMPLEMENT_NETWORKCLASS_ALIASED( MSLTurretHead, DT_MSLTurretHead );

BEGIN_NETWORK_TABLE( CMSLTurretHead, DT_MSLTurretHead )
END_NETWORK_TABLE()

CMSLTurretHead::CMSLTurretHead() {}
CMSLTurretHead::~CMSLTurretHead() {}

CSeekerMissile::CSeekerMissile() {}
CSeekerMissile::~CSeekerMissile() {

#ifndef CLIENT_DLL
	StopSound(SEEKER_MISSILE_SOUND);
	StopParticleEffects(this);
#endif

}


#ifndef CLIENT_DLL

BEGIN_DATADESC(CSeekerMissile)
	DEFINE_THINKFUNC(SeekThink),
END_DATADESC()

BEGIN_DATADESC(CMSLTurretEntity)
	DEFINE_THINKFUNC(DetectThink),
END_DATADESC()

void CMSLTurretEntity::Precache(void) {
	UTIL_PrecacheOther("msl_turret_head");
	PrecacheScriptSound(TURRET_ACQUIRE_TARGET_SND);
	PrecacheScriptSound(TURRET_LOSE_TARGET_SND);
	PrecacheScriptSound(MSL_TURRET_FIRE_SND);
	BaseClass::Precache();
}

void CMSLTurretEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	UTIL_SetSize(this, MSL_TURRET_HULL_MIN, MSL_TURRET_HULL_MAX);
	//SetSolid(SOLID_VPHYSICS);
	//SetCollisionGroup(COLLISION_GROUP_PLAYER);
	
	turretHead = (CMSLTurretHead *)CreateEntityByName("msl_turret_head");
	if (turretHead) {
		turretHead->SetAbsOrigin(GetAbsOrigin() + Vector(0,0,12));
		turretHead->SetAbsAngles(vec3_angle);
		turretHead->SetParent(this);
		turretHead->ChangeTeam(TEAM_HUMANS);
		DispatchSpawn(turretHead);

		SetThink(&CMSLTurretEntity::DetectThink);
		SetNextThink(gpGlobals->curtime + MSL_TURRET_DETECT_INTERVAL);
	}
}

void CMSLTurretEntity::DetectThink(void) {
	CBaseEntity *ent;
	trace_t tr;

	SetNextThink(gpGlobals->curtime + MSL_TURRET_DETECT_INTERVAL);
	
	if (active == true) {
		for (CEntitySphereQuery sphere(GetAbsOrigin(), MSL_TURRET_DETECT_RADIUS); 
			(ent = sphere.GetCurrentEntity()) != NULL; 
			sphere.NextEntity()) 
		{
			UTIL_TraceLine
			(
				GetAbsOrigin(),
				ent->GetAbsOrigin(),
				MASK_SOLID,
				this,
				COLLISION_GROUP_NONE,
				&tr
			);
		
			if (tr.DidHit() && tr.m_pEnt) {
				//
				// tell the turret head about the possible target
				//
				if (turretHead->HasTarget() == false &&
					turretHead->CanAcquireTarget(tr.m_pEnt)) {
						turretHead->EngageTarget(tr.m_pEnt);
				}
			}
		}
	}
}

int CMSLTurretEntity::OnTakeDamage(const CTakeDamageInfo &info) {
	int dmgtype;
	CTakeDamageInfo newinfo = info;

	dmgtype = info.GetDamageType();

	//
	// make sure the turret takes damage from bullets
	//
	if (dmgtype & DMG_BULLET) {
		newinfo.SetDamageType(DMG_BULLET | DMG_ALWAYSGIB);
	}

	return BaseClass::OnTakeDamage(newinfo);
}

///////////////////////////////////
// TURRET HEAD
///////////////////////////////////

BEGIN_DATADESC(CMSLTurretHead)
	DEFINE_THINKFUNC(TrackTargetThink),
END_DATADESC()

void CMSLTurretHead::Precache(void) {
	PrecacheModel(MSL_TURRET_HEAD_MODEL);
	BaseClass::Precache();
}

void CMSLTurretHead::SetCreator(CBaseEntity *c) {
	creator = c;
}

CBaseEntity *CMSLTurretHead::GetCreator(void) {
	return creator;
}

void CMSLTurretHead::Spawn(void) {
	Precache();
	SetModel(MSL_TURRET_HEAD_MODEL);
	BaseClass::Spawn();

	SetMoveType(MOVETYPE_VPHYSICS);
	SetCollisionGroup(COLLISION_GROUP_NONE);
	SetSolid(SOLID_NONE);

	m_iAmmoType	= GetAmmoDef()->Index("AR2");
	
	//SetLocalOrigin( vec3_origin );
	SetLocalAngles( vec3_angle );

	m_yawCenter			= GetLocalAngles().y;
	m_yawCenterWorld	= GetAbsAngles().y;
	m_pitchCenter		= GetLocalAngles().x;
	m_pitchCenterWorld	= GetAbsAngles().y;
	m_vTargetPosition	= vec3_origin;

	//m_barrelPos = vec3_origin;
	m_iszBarrelAttachment = "muzzle";
	CBaseAnimating *pAnim = GetBaseAnimating();

	m_nBarrelAttachment = pAnim->LookupAttachment( m_iszBarrelAttachment );

	Vector vecWorldBarrelPos;
	QAngle worldBarrelAngle;

	pAnim->GetAttachment( m_nBarrelAttachment, vecWorldBarrelPos, worldBarrelAngle );
	VectorITransform( vecWorldBarrelPos, EntityToWorldTransform( ), m_barrelPos );


	m_yawRate = MSL_YAW_RATE;
	m_yawRange = MSL_YAW_RANGE;
	m_yawTolerance = MSL_YAW_TOL;

	m_pitchRate = MSL_PITCH_RATE;
	m_pitchRange = MSL_PITCH_RANGE;
	m_pitchTolerance = MSL_PITCH_TOL;

	m_flNextFireTime = 0.0f;

	creator = NULL;
	target = NULL;

	UpdateMatrix();

	SetThink(&CMSLTurretHead::TrackTargetThink);
	SetNextThink(gpGlobals->curtime + MSL_NO_TARGET_INTERVAL);
}

bool CMSLTurretHead::HasTarget(void) {
	if (target)
		return true;
	return false;
}

int CMSLTurretHead::OnTakeDamage(const CTakeDamageInfo &info) {
	GetParent()->OnTakeDamage(info);
	return 0;
}

bool CMSLTurretHead::RotateToAngles( const QAngle &angles, float *pDistX, float *pDistY ) {
	bool bClamped = false;

	//
	// see how far the requested angle is from the "center" angles
	// so that they can be clamped if the rotation hits
	// range + Tolerance on either of the axes
	//
	float offsetY = UTIL_AngleDistance( angles.y, m_yawCenter );
	float offsetX = UTIL_AngleDistance( angles.x, m_pitchCenter );

	float flActualYaw = m_yawCenter + offsetY;
	float flActualPitch = m_pitchCenter + offsetX;

	if ( ( fabs( offsetY ) > m_yawRange + m_yawTolerance ) ||
		 ( fabs( offsetX ) > m_pitchRange + m_pitchTolerance ) )
	{
		//
		// limit rotation to the specified ranges in pitch/yaw
		//
		flActualYaw = clamp( flActualYaw, m_yawCenter - m_yawRange, m_yawCenter + m_yawRange );
		flActualPitch = clamp( flActualPitch, m_pitchCenter - m_pitchRange, m_pitchCenter + m_pitchRange );

		bClamped = true;
	}

	// Get at the angular vel
	QAngle vecAngVel = GetLocalAngularVelocity();

	// Move toward target at rate or less
	float distY = UTIL_AngleDistance( flActualYaw, GetLocalAngles().y );
	vecAngVel.y = distY * 10;
	vecAngVel.y = clamp( vecAngVel.y, -m_yawRate, m_yawRate );

	// Move toward target at rate or less
	float distX = UTIL_AngleDistance( flActualPitch, GetLocalAngles().x );
	vecAngVel.x = distX  * 10;
	vecAngVel.x = clamp( vecAngVel.x, -m_pitchRate, m_pitchRate );

	// How exciting! We're done
	//SetLocalAngularVelocity( vecAngVel );

	//
	// How did FuncTank use Angular velocity..
	// if firing stops the rotation because the player is
	// in the gun sights.. it still doesn't line up correctly
	//
	SetLocalAngles(QAngle(GetLocalAngles().x + distX, GetLocalAngles().y + distY, 0.0f));

	if (pDistX && pDistY) {
		*pDistX = distX;
		*pDistY = distY;
	}

	return bClamped;
}

//-----------------------------------------------------------------------------
// Purpose: Aim the offset barrel at a position in parent space
// Input  : parentTarget - the position of the target in parent space
// Output : Vector - angles in local space
//-----------------------------------------------------------------------------
QAngle CMSLTurretHead::AimBarrelAt(const Vector &parentTarget) {
	Vector target = parentTarget - GetLocalOrigin();
	float quadTarget = target.LengthSqr();
	float quadTargetXY = target.x*target.x + target.y*target.y;

	// Target is too close!  Can't aim at it
	if ( quadTarget <= m_barrelPos.LengthSqr() ) {
		return GetLocalAngles();
	} else {
		// We're trying to aim the offset barrel at an arbitrary point.
		// To calculate this, I think of the target as being on a sphere with 
		// it's center at the origin of the gun.
		// The rotation we need is the opposite of the rotation that moves the target 
		// along the surface of that sphere to intersect with the gun's shooting direction
		// To calculate that rotation, we simply calculate the intersection of the ray 
		// coming out of the barrel with the target sphere (that's the new target position)
		// and use atan2() to get angles

		// angles from target pos to center
		float targetToCenterYaw = atan2( target.y, target.x );
		float centerToGunYaw = atan2( m_barrelPos.y, sqrt( quadTarget - (m_barrelPos.y*m_barrelPos.y) ) );

		float targetToCenterPitch = atan2( target.z, sqrt( quadTargetXY ) );
		float centerToGunPitch = atan2( -m_barrelPos.z, sqrt( quadTarget - (m_barrelPos.z*m_barrelPos.z) ) );
		return QAngle(-RAD2DEG(targetToCenterPitch+centerToGunPitch), RAD2DEG( targetToCenterYaw + centerToGunYaw ), 0);
	}
}

Vector CMSLTurretHead::WorldBarrelPosition(void) {
	EntityMatrix tmp;
	tmp.InitFromEntity(this);
	return tmp.LocalToWorld(m_barrelPos);
}

void CMSLTurretHead::AimAtTarget( CBaseEntity *ent ) {
	float distX, distY;
	trace_t tr;
	QAngle angles;
	Vector barrelEnd;
	Vector worldTargetPosition;
	Vector vecAimOrigin;
	Vector vecLocalOrigin;

	barrelEnd = WorldBarrelPosition();
	worldTargetPosition = ent->BodyTarget(GetAbsOrigin(), false);
	vecAimOrigin = worldTargetPosition;

	// Convert targetPosition to parent
	vecLocalOrigin = m_parentMatrix.WorldToLocal( vecAimOrigin );
	angles = AimBarrelAt(vecLocalOrigin);
	
	(void)RotateToAngles(angles, &distX, &distY);
	
	if (m_flNextFireTime < gpGlobals->curtime &&
		fabs(distX) < m_pitchTolerance &&
		fabs(distY) < m_yawTolerance)
	{
		ShootAt(ent);
	}
}

void CMSLTurretHead::ShootAt(CBaseEntity *ent) {
	Vector dir;
	Vector endpos;
	Vector shootpos;
	CSeekerMissile *msl;
	trace_t tr;

	m_flNextFireTime = gpGlobals->curtime + MSL_TURRET_SHOT_DELAY;

	shootpos = WorldBarrelPosition();
	endpos = ent->BodyTarget(shootpos, false);
	dir = endpos - shootpos;

	UTIL_TraceLine(shootpos, endpos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if (tr.DidHit() && tr.m_pEnt) {
		if (tr.m_pEnt != target && 
			tr.m_pEnt->ClassMatches("msl_turret_head") == false &&
			tr.m_pEnt->ClassMatches("ent_msl_turret") == false) {
			return;
		}
	}

	(void)VectorNormalize(dir);

	msl = (CSeekerMissile *)CBaseEntity::Create("ent_seeker_msl", shootpos, GetAbsAngles(), GetCreator());
	if (msl) {
		dir *= SEEKER_MISSILE_SPEED;
		msl->SetTargetPlayer(ent);
		msl->SetAbsVelocity(dir);
	}

	EmitSound(MSL_TURRET_FIRE_SND);

	//DebugDrawLine(GetAbsOrigin(), ent->BodyTarget(GetAbsOrigin(), false), 255, 0, 0, false, 0.5f);
}

void CMSLTurretHead::UpdateMatrix(void) {
	m_parentMatrix.InitFromEntity( GetParent(), GetParentAttachment() );
}

void CMSLTurretHead::EngageTarget(CBaseEntity *ent) {
	target = ent;
	SetNextThink(gpGlobals->curtime);
	EmitSound(TURRET_ACQUIRE_TARGET_SND);
}

bool CMSLTurretHead::CanAcquireTarget(CBaseEntity *ent) {
	Vector toTarget;
	CHL2MP_Player *p;

	if (ent == NULL)
		return false;

	if ((p = ToHL2MPPlayer(ent)) == NULL)
		return false;

	if (ent->IsAlive() == false)
		return false;

	if (p->GetTeamNumber() != TEAM_SPIDERS)
		return false;

	toTarget = GetAbsOrigin() - ent->BodyTarget(GetAbsOrigin(), false);
	if (toTarget.Length() > SMG_TURRET_DETECT_RADIUS)
		return false;

	return true;
}

void CMSLTurretHead::TrackTargetThink(void) {
	Vector toTarget;

	SetNextThink(gpGlobals->curtime + MSL_HAVE_TARGET_INTERVAL);
	SetLocalAngularVelocity(vec3_angle);

	if (target == NULL) {
		SetNextThink(gpGlobals->curtime + MSL_NO_TARGET_INTERVAL);
		return;
	}

	//
	// if the target has moved out of range/died/change team
	// set target to NULL and return.
	// play a sound to reflect the new state
	//
	if (target && CanAcquireTarget(target) == false) {
			target = NULL;
			SetLocalAngularVelocity(vec3_angle);
			SetNextThink(gpGlobals->curtime + MSL_NO_TARGET_INTERVAL);
			EmitSound(TURRET_LOSE_TARGET_SND);
			return;
	}

	AimAtTarget(target);
	SetMoveDoneTime(0.1f);
}


////////////////////////////////////
// Seeker Missile
////////////////////////////////////

void CSeekerMissile::Spawn(void) {
	Precache();
	BaseClass::Spawn();
	target = NULL;

	UTIL_SetSize(this, -Vector(3,3,3), Vector(3,3,3));

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetMoveType(MOVETYPE_FLY);
	SetModel(SEEKER_MISSILE_MODEL);

	AddEffects(EF_NOSHADOW);

	SetThink(&CSeekerMissile::SeekThink);
	SetNextThink(gpGlobals->curtime);

	SetTouch(&CSeekerMissile::MissileTouch);

	m_flSelfDestruct = gpGlobals->curtime + 3.0f;
	
	DispatchParticleEffect(SEEKER_MISSILE_PARTICLE, PATTACH_ABSORIGIN_FOLLOW, this);

	EmitSound(SEEKER_MISSILE_SOUND);

}

void CSeekerMissile::Precache(void) {
	PrecacheModel(SEEKER_MISSILE_MODEL);
	PrecacheParticleSystem(SEEKER_MISSILE_PARTICLE);
	PrecacheScriptSound(SEEKER_MISSILE_SOUND);
	BaseClass::Precache();
}

void CSeekerMissile::SeekThink(void) {
    float dotDist;
	float flSpeed;
	float flDist;
	float flHomingSpeed;
	Vector origin;
	Vector targetPos;
	Vector vTargetDir;
    Vector vecLaserDotPosition;
	Vector vDir;
    Vector vNewVelocity;
	QAngle finalAngles;

	if (m_flSelfDestruct < gpGlobals->curtime) {
		UTIL_Remove(this);
		return;
	}

	if (target == NULL)
		return;

	origin = GetAbsOrigin();
	targetPos = target->BodyTarget(origin, false);
	dotDist = (origin - targetPos).Length();
	flHomingSpeed = SEEKER_MISSILE_SEEK_SPD;

    SetNextThink( gpGlobals->curtime );

    if (IsSimulatingOnAlternateTicks())
		flHomingSpeed *= 2;

    VectorSubtract(targetPos, origin, vTargetDir);
    flDist = VectorNormalize(vTargetDir);

    vDir = GetAbsVelocity();
    flSpeed = VectorNormalize(vDir);
    vNewVelocity = vDir;

    if (gpGlobals->frametime > 0.0f) {
        if (flSpeed != 0) {
            vNewVelocity = ( flHomingSpeed * vTargetDir ) + ( ( 1 - flHomingSpeed ) * vDir );

            // This computation may happen to cancel itself out exactly. If so, slam to targetdir.
            if ( VectorNormalize( vNewVelocity ) < 1e-3 ) {
                vNewVelocity = (flDist != 0) ? vTargetDir : vDir;
            }
        } else {
            vNewVelocity = vTargetDir;
        }
    }

    
    VectorAngles(vNewVelocity, finalAngles);
    SetAbsAngles(finalAngles);

    vNewVelocity *= flSpeed;
    SetAbsVelocity(vNewVelocity);

    if (GetAbsVelocity() == vec3_origin) {
		UTIL_Remove(this);
        return;
    }
}

void CSeekerMissile::SetTargetPlayer(CBaseEntity *p) {
	target = p;
}

void CSeekerMissile::MissileTouch(CBaseEntity *other) {
	trace_t tr;
	Vector forward;

	if (other->ClassMatches("msl_turret_head") || other->ClassMatches("ent_msl_turret"))
		return;

	if (other->GetTeamNumber() == TEAM_SPIDERS) {
		tr = BaseClass::GetTouchTrace();
		forward = tr.endpos - tr.startpos;

		SetSolid(SOLID_NONE);

		CTakeDamageInfo info
		(
			this,
			GetOwnerEntity(),
			GetAbsVelocity(),
			GetAbsOrigin(),
			SEEKER_MISSILE_DMG,
			DMG_BULLET | DMG_ALWAYSGIB
		);

		info.SetAmmoType(GetAmmoDef()->Index("turret_missile"));

		other->DispatchTraceAttack(info, forward, &tr);
		ApplyMultiDamage();

		
		UTIL_Remove(this);
		return;
	}

	// allow it to pass through water
	if (other->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;

	UTIL_Remove(this);
}





#else // below is client_dll code

int CMSLTurretHead::DrawModel(int flags) {
	C_HL2MP_Player *local;

	shouldGlow = false;

	local = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (local && local->GetTeamNumber() == TEAM_SPIDERS) {
		shouldGlow = true;
		if (GetTeamNumber() == TEAM_HUMANS && local->m_iClassNumber == CLASS_COMMANDO_IDX
			&& GetAbsVelocity() == vec3_origin) {
				shouldGlow = false;
		}
	}

	return BaseClass::DrawModel(flags);
}

#endif
