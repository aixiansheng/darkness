//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_GUARDIAN_H
#define GRENADE_GUARDIAN_H
#pragma once

#include "basegrenade_shared.h"

class CBaseGrenade;
struct edict_t;

class CGrenadeGuardian : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeGuardian, CBaseGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
	~CGrenadeGuardian( void );

public:
	void	Spawn( void );
	void	Precache( void );
	void	SetTimer( float detonateDelay );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	BlipSound() { EmitSound( "Grenade.Blip" ); }
	void	ExplodeThink(void);
	void	TriggerThink(void);
	void	Detonate(void);
	void	Explode( trace_t *pTrace, int bitsDamageType );
	void	SetSticky(void);
	void	StickTouch(CBaseEntity *other);
	void	ShootSpike(Vector origin, Vector v);

	// this function only used in episodic.
#ifdef HL2_EPISODIC
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
#endif 

protected:

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	sticky;
	trace_t	surface_trace;

};

CBaseGrenade *GuardianGren_Create( const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner, float timer, bool sticky);

#endif // GRENADE_GUARDIAN_H
