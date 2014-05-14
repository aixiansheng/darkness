#ifndef MSL_TURRET_H
#define MSL_TURRET_H

#include "cbase.h"
#include "human_materiel.h"

#define MSL_TURRET_HEAD_MODEL "models/turret_head.mdl"
#define MSL_TURRET_DETECT_INTERVAL 0.3f
#define MSL_TURRET_DETECT_RADIUS 480
#define MSL_TURRET_SHOT_DELAY 1.5f

#define MSL_TURRET_FIRE_SND	"Turret.FireMSL"

#ifdef CLIENT_DLL

	#include "c_hl2mp_player.h"
	#include "c_baseplayer.h"
	#include "model_types.h"

	#define CMSLTurretEntity C_MSLTurretEntity
	#define CMSLTurretHead C_MSLTurretHead
	#define CSeekerMissile C_SeekerMissile
#else

	#include "world.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
	#include "item_info.h"
	#include "ammodef.h"
	#include "particle_parse.h"

#endif

#define SEEKER_MISSILE_DMG			70
#define SEEKER_MISSILE_THINK_INT	0.1f
#define SEEKER_MISSILE_SEEK_SPD		0.125f
#define SEEKER_MISSILE_MODEL		"models/weapons/w_missile.mdl"
#define SEEKER_MISSILE_SPEED		600.0f

#define SEEKER_MISSILE_SOUND		"TurretMissile.Loop"
#define SEEKER_MISSILE_PARTICLE		"missile_trail"

class CSeekerMissile : public CBaseAnimating {
	public:
		DECLARE_CLASS(CSeekerMissile, CBaseAnimating);
		DECLARE_NETWORKCLASS();

		CSeekerMissile();
		~CSeekerMissile();

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);
		void SeekThink(void);
		void MissileTouch(CBaseEntity *other);
		void SetTargetPlayer(CBaseEntity *p);

	private:
		CBaseEntity *target;
		float m_flSelfDestruct;
#endif

};

class CMSLTurretHead : public CBaseAnimating {
	public:
		DECLARE_CLASS(CMSLTurretHead, CBaseAnimating);
		DECLARE_NETWORKCLASS(); 
				
		CMSLTurretHead();
		~CMSLTurretHead();

#ifndef CLIENT_DLL

		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);

		bool RotateToAngles(const QAngle &angles, float *pDistX, float *pDistY);
		void AimAtTarget(CBaseEntity *ent);
		QAngle AimBarrelAt(const Vector &parentTarget);
		Vector WorldBarrelPosition(void);
		void UpdateMatrix(void);
		void EngageTarget(CBaseEntity *e);
		void TrackTargetThink(void);
		void ShootAt(CBaseEntity *ent);
		bool CanAcquireTarget(CBaseEntity *ent);
		bool HasTarget(void);

		virtual int OnTakeDamage(const CTakeDamageInfo &info);

		void SetCreator(CBaseEntity *creator);
		CBaseEntity *GetCreator(void);

	private:
		
		float m_yawRate;
		float m_yawRange;
		float m_yawCenter;
		float m_yawCenterWorld;
		float m_yawTolerance;

		float m_pitchRate;
		float m_pitchRange;
		float m_pitchCenter;
		float m_pitchCenterWorld;
		float m_pitchTolerance;

		float m_flNextFireTime;

		const char *m_iszBarrelAttachment;
		int m_nBarrelAttachment;

		int m_iAmmoType;

		Vector m_vTargetPosition;
		Vector m_barrelPos;
		EntityMatrix m_parentMatrix;

		CBaseEntity *target;

		CBaseEntity *creator;

#else // client_dll code below
	public:
		virtual int DrawModel(int flags);
#endif

};


class CMSLTurretEntity : public CHumanMateriel {
	public:
		DECLARE_CLASS(CMSLTurretEntity, CHumanMateriel);
		DECLARE_NETWORKCLASS(); 
				
		CMSLTurretEntity();
		~CMSLTurretEntity();

#ifndef CLIENT_DLL

		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);
		void DetectThink(void);
		virtual int OnTakeDamage(const CTakeDamageInfo &info);

#endif
	
	private:
		
		CMSLTurretHead *turretHead;

};

#endif
