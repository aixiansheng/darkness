//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_SMK_H
#define GRENADE_SMK_H
#pragma once

class CBaseGrenade;
struct edict_t;

CBaseGrenade *SmkGren_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned );
bool	SmkGren_WasPunted( const CBaseEntity *pEntity );
bool	SmkGren_WasCreatedByCombine( const CBaseEntity *pEntity );

#endif // GRENADE_FRAG_H
