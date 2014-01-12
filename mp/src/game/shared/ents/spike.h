#ifndef __SPIKE_H__
#define __SPIKE_H__

#include "cbase.h"

#define SPIKE_MODEL "models/spike.mdl"
#define SPIKE_DAMAGE 75.0f

class CSpike : public CBaseAnimating {

	DECLARE_CLASS( CSpike, CBaseAnimating );

#ifndef CLIENT_DLL
	//DECLARE_DATADESC();
					
	~CSpike( void );

public:
	void	Spawn(void);
	void	Precache(void);
	void	SpikeTouch(CBaseEntity *other);
	void	FireAt(Vector v);

#endif

public:
	float	spikeDamage;

};


#ifndef CLIENT_DLL
CSpike *Spike_Create( const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner, float damage, bool fire_now);
#endif

#endif
