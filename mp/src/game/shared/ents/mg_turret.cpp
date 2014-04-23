#include "cbase.h"
#include "mg_turret.h"
#include "item_info.h"
#include "class_info.h"

#define SMG_PITCH_RATE	100
#define SMG_PITCH_RANGE	50
#define SMG_PITCH_TOL	10

#define SMG_YAW_RATE	SMG_PITCH_RATE
#define SMG_YAW_RANGE	180
#define SMG_YAW_TOL		SMG_PITCH_TOL

#define SMG_NO_TARGET_INTERVAL 0.1f
#define SMG_HAVE_TARGET_INTERVAL 0.05f


LINK_ENTITY_TO_CLASS (ent_mg_turret, CMGTurretEntity );
IMPLEMENT_NETWORKCLASS_ALIASED( MGTurretEntity, DT_MGTurretEntity );

BEGIN_NETWORK_TABLE( CMGTurretEntity, DT_MGTurretEntity )
END_NETWORK_TABLE()

CMGTurretEntity::CMGTurretEntity() : CHumanMateriel(&human_items[ITEM_SMG_TURRET_IDX]) {}
CMGTurretEntity::~CMGTurretEntity() {}


LINK_ENTITY_TO_CLASS (smg_turret_head, CMGTurretHead );
IMPLEMENT_NETWORKCLASS_ALIASED( MGTurretHead, DT_MGTurretHead );

BEGIN_NETWORK_TABLE( CMGTurretHead, DT_MGTurretHead )
END_NETWORK_TABLE()

CMGTurretHead::CMGTurretHead() {}
CMGTurretHead::~CMGTurretHead() {}



#ifndef CLIENT_DLL

BEGIN_DATADESC(CMGTurretEntity)
	DEFINE_THINKFUNC(DetectThink),
END_DATADESC()

void CMGTurretEntity::Precache(void) {
	UTIL_PrecacheOther("smg_turret_head");
	PrecacheScriptSound(TURRET_ACQUIRE_TARGET_SND);
	PrecacheScriptSound(TURRET_LOSE_TARGET_SND);
	BaseClass::Precache();
}

void CMGTurretEntity::Spawn(void) {
	Precache();
	BaseClass::Spawn();

	UTIL_SetSize(this, SMG_TURRET_HULL_MIN, SMG_TURRET_HULL_MAX);
	//SetSolid(SOLID_VPHYSICS);
	//SetCollisionGroup(COLLISION_GROUP_PLAYER);
	
	turretHead = (CMGTurretHead *) CreateEntityByName("smg_turret_head");
	turretHead->SetAbsOrigin(GetAbsOrigin() + Vector(0,0,12));
	turretHead->SetAbsAngles(vec3_angle);
	turretHead->SetParent(this);
	turretHead->ChangeTeam(TEAM_HUMANS);
	DispatchSpawn(turretHead);

	SetThink(&CMGTurretEntity::DetectThink);
	SetNextThink(gpGlobals->curtime + SMG_TURRET_DETECT_INTERVAL);
}

void CMGTurretEntity::DetectThink(void) {
	CBaseEntity *ent;
	trace_t tr;

	SetNextThink(gpGlobals->curtime + SMG_TURRET_DETECT_INTERVAL);
	
	if (active == true) {
		for (CEntitySphereQuery sphere(GetAbsOrigin(), SMG_TURRET_DETECT_RADIUS); 
			(ent = sphere.GetCurrentEntity()) != NULL; 
			sphere.NextEntity()) 
		{
			UTIL_TraceLine(GetAbsOrigin(), ent->GetAbsOrigin(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		
			if (tr.DidHit()) {

				if (tr.m_pEnt->IsPlayer() && tr.m_pEnt->GetTeamNumber() == TEAM_SPIDERS && tr.m_pEnt->IsAlive()) {
					// do somethinf about the target
					turretHead->PossibleTarget(tr.m_pEnt);
					//Warning("Aiming At Target\n");
				}
			}
		}
	}
}

int CMGTurretEntity::OnTakeDamage(const CTakeDamageInfo &info) {
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

BEGIN_DATADESC(CMGTurretHead)
	DEFINE_THINKFUNC(TrackTargetThink),
END_DATADESC()

void CMGTurretHead::Precache(void) {
	PrecacheModel(SMG_TURRET_HEAD_MODEL);
	BaseClass::Precache();
}

void CMGTurretHead::Spawn(void) {
	Precache();
	SetModel(SMG_TURRET_HEAD_MODEL);
	BaseClass::Spawn();

	SetMoveType(MOVETYPE_VPHYSICS);
	SetCollisionGroup(COLLISION_GROUP_NONE);
	SetSolid(SOLID_NONE);

	m_iAmmoType	= GetAmmoDef()->Index("turret_bullet");
	
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


	m_yawRate = SMG_YAW_RATE;
	m_yawRange = SMG_YAW_RANGE;
	m_yawTolerance = SMG_YAW_TOL;

	m_pitchRate = SMG_PITCH_RATE;
	m_pitchRange = SMG_PITCH_RANGE;
	m_pitchTolerance = SMG_PITCH_TOL;

	m_flNextFireTime = 0.0f;

	target = NULL;

	UpdateMatrix();

	SetThink(&CMGTurretHead::TrackTargetThink);
	SetNextThink(gpGlobals->curtime + SMG_NO_TARGET_INTERVAL);
}

int CMGTurretHead::OnTakeDamage(const CTakeDamageInfo &info) {
	GetParent()->OnTakeDamage(info);
	return 0;
}

bool CMGTurretHead::RotateToAngles( const QAngle &angles, float *pDistX, float *pDistY ) {
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

	if ( pDistX && pDistY )
	{
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
QAngle CMGTurretHead::AimBarrelAt(const Vector &parentTarget) {
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
		return QAngle( -RAD2DEG(targetToCenterPitch+centerToGunPitch), RAD2DEG( targetToCenterYaw + centerToGunYaw ), 0 );
	}
}

Vector CMGTurretHead::WorldBarrelPosition(void) {
	EntityMatrix tmp;
	tmp.InitFromEntity(this);
	return tmp.LocalToWorld(m_barrelPos);
}

void CMGTurretHead::AimAtTarget( CBaseEntity *ent ) {
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
	angles = AimBarrelAt( vecLocalOrigin );
	
	(void)RotateToAngles( angles, &distX, &distY );
	
	if (m_flNextFireTime < gpGlobals->curtime &&
		fabs(distX) < m_pitchTolerance &&
		fabs(distY) < m_yawTolerance)
	{
		ShootAt(ent);
	}
}

void CMGTurretHead::ShootAt(CBaseEntity *ent) {
	Vector dir;
	Vector endpos;
	Vector shootpos;

	trace_t tr;

	m_flNextFireTime = gpGlobals->curtime + SMG_TURRET_SHOT_DELAY;

	EmitSound( "Weapon_functank.Single" );

	shootpos = WorldBarrelPosition();
	endpos = ent->BodyTarget(shootpos, false);
	dir = endpos - shootpos;

	UTIL_TraceLine(shootpos, endpos, MASK_SOLID, GetParent(), COLLISION_GROUP_NONE, &tr);
	CTakeDamageInfo info(this, GetParent(), SMG_TURRET_DAMAGE, DMG_BULLET);
	info.SetAmmoType(m_iAmmoType);

	CalculateMeleeDamageForce(&info, dir, endpos);
	ent->DispatchTraceAttack(info, dir, &tr);
	ApplyMultiDamage();

}

void CMGTurretHead::UpdateMatrix(void) {
	m_parentMatrix.InitFromEntity( GetParent(), GetParentAttachment() );
}

//
// Only update the gun target if the gun needs a target
//
void CMGTurretHead::PossibleTarget(CBaseEntity *ent) {

	if (target == NULL) {
		target = ent;
		SetNextThink(gpGlobals->curtime);
		EmitSound(TURRET_ACQUIRE_TARGET_SND);
	}
}

void CMGTurretHead::TrackTargetThink(void) {
	Vector toTarget;
	CMGTurretEntity *parent;

	parent = dynamic_cast<CMGTurretEntity *>(GetParent());
	if (parent && parent->active == false)
		return;
	
	SetNextThink(gpGlobals->curtime + SMG_HAVE_TARGET_INTERVAL);
	
	SetLocalAngularVelocity(vec3_angle);

	if (target == NULL) {
		SetNextThink(gpGlobals->curtime + SMG_NO_TARGET_INTERVAL);
		return;
	}

	toTarget = GetAbsOrigin() - target->BodyTarget(GetAbsOrigin(), false);
	if (toTarget.Length() > SMG_TURRET_DETECT_RADIUS || !target->IsAlive()) {
		target = NULL;
		SetLocalAngularVelocity(vec3_angle);
		SetNextThink(gpGlobals->curtime + SMG_NO_TARGET_INTERVAL);
		EmitSound(TURRET_LOSE_TARGET_SND);
		return;
	}

	AimAtTarget(target);
	SetMoveDoneTime(0.1f);
}

#else // below is client_dll code

int CMGTurretHead::DrawModel(int flags) {
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