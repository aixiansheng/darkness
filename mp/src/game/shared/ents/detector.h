#ifndef DETECTOR_H
#define DETECTOR_H

#include "cbase.h"
#include "human_materiel.h"

#ifdef CLIENT_DLL
	#define CDetectorEntity C_DetectorEntity
#else
	#include "team_spawnpoint.h"
	#include "item_info.h"
	#include "team_spawnpoint.h"
	#include "hl2mp_gamerules.h"
	#include "team.h"
	#include "item_info.h"
	#include "explode.h"
#endif

#define DETECTOR_RADIUS 800
#define DETECTOR_THINK_INTERVAL 1.5f
#define DETECTOR_ARM_TIME 2.0f
#define DETECTOR_SOUND "Detector.Ping"

class CDetectorEntity : public CHumanMateriel {
	public:
		DECLARE_CLASS(CDetectorEntity, CHumanMateriel);
		DECLARE_NETWORKCLASS();

		CDetectorEntity(void);
		~CDetectorEntity(void);

#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		void Precache(void);
		void Spawn(void);
		void SetupThink(void);
		void DetectThink(void);
		void SolidThink(void);

#endif

};

#endif
