#ifndef MG_TURRET_H
#define MG_TURRET_H

#include "cbase.h"
#include "human_materiel.h"

#define SMG_TURRET_HEAD_MODEL "models/turret_head.mdl"
#define SMG_TURRET_DETECT_INTERVAL 0.3f
#define SMG_TURRET_DETECT_RADIUS 400
#define SMG_TURRET_SHOT_DELAY 0.11f
#define SMG_TURRET_DAMAGE 5

#define TURRET_ACQUIRE_TARGET_SND	"Turret.AcquirePlayer"
#define TURRET_LOSE_TARGET_SND		"Turret.LosePlayer"

#define ACK_BEEP_INT 2.0f

#ifdef CLIENT_DLL

	#include "c_hl2mp_player.h"
	#include "c_baseplayer.h"
	#include "model_types.h"

	#define CMGTurretEntity C_MGTurretEntity
	#define CMGTurretHead C_MGTurretHead
#else

	#include "hl2mp_gamerules.h"
	#include "team.h"
	#include "item_info.h"
	#include "ammodef.h"

#endif


class CMGTurretHead : public CBaseAnimating {
	public:
		DECLARE_CLASS(CMGTurretHead, CBaseAnimating);
		DECLARE_NETWORKCLASS(); 
				
		CMGTurretHead();
		~CMGTurretHead();

#ifndef CLIENT_DLL

		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);

		bool RotateToAngles(const QAngle &angles, float *pDistX, float *pDistY);
		void AimAtTarget(CBaseEntity *ent);
		QAngle AimBarrelAt(const Vector &parentTarget);
		Vector WorldBarrelPosition(void);
		void UpdateMatrix(void);

		void PossibleTarget(CBaseEntity *e);

		void TrackTargetThink(void);

		void ShootAt(CBaseEntity *ent);

		virtual int OnTakeDamage(const CTakeDamageInfo &info);

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


class CMGTurretEntity : public CHumanMateriel {
	public:
		DECLARE_CLASS(CMGTurretEntity, CHumanMateriel);
		DECLARE_NETWORKCLASS(); 
				
		CMGTurretEntity();
		~CMGTurretEntity();

#ifndef CLIENT_DLL

		DECLARE_DATADESC();

		void Spawn(void);
		void Precache(void);
		void DetectThink(void);
		virtual int OnTakeDamage(const CTakeDamageInfo &info);

#endif
	
	private:
		
		CMGTurretHead *turretHead;

};

#endif