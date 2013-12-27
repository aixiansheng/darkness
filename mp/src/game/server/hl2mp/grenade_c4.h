//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_C4_H
#define GRENADE_C4_H
#pragma once

#include "basegrenade_shared.h"
#include "Sprite.h"

class CBaseGrenade;
struct edict_t;

class CGrenadeC4 : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeC4, CBaseGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
	~CGrenadeC4( void );

public:
	void	Spawn( void );
	void	Precache( void );
	void	SetTimer( float detonateDelay );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	ExplodeThink(void);
	void	Detonate(void);
	void	Explode( trace_t *pTrace, int bitsDamageType );
	
	// just override Use, because CBaseGrenade does so,
	// and as a result, grenades that call SetUse, won't see
	// the intended effect :(
	void	Use(CBaseEntity *activator, CBaseEntity *caller, USE_TYPE useType, float value);

#ifndef CLIENT_DLL
	virtual int ObjectCaps(void);
#endif

protected:

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	use_ok;

	void	CreateEffects(void);
	CHandle<CSprite>	m_hGlowSprite;
};

CBaseGrenade *C4_Create( const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner, float timer);

#endif // GRENADE_C4_H
