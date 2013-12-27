//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_ACID_H
#define GRENADE_ACID_H
#pragma once

class CBaseGrenade;
struct edict_t;

CBaseGrenade *AcidGren_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned );
bool	AcidGren_WasPunted( const CBaseEntity *pEntity );
bool	AcidGren_WasCreatedByCombine( const CBaseEntity *pEntity );

#endif // GRENADE_ACID_H
