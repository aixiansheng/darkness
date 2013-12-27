#ifndef WEAPON_GRAPPLE_H
#define WEAPON_GRAPPLE_H
 
#ifdef _WIN32
#pragma once
#endif
 
#include "weapon_hl2mpbasehlmpcombatweapon.h"

 
#ifndef CLIENT_DLL
	#include "props.h"
#endif
 
#ifndef CLIENT_DLL

class CWeaponGrapple;

//-----------------------------------------------------------------------------
// Grapple Hook
//-----------------------------------------------------------------------------
class CGrapplingHook : public CBaseCombatCharacter {
	DECLARE_CLASS( CGrapplingHook, CBaseCombatCharacter );
 
public:
	CGrapplingHook() { };
	~CGrapplingHook();
 
	Class_T Classify( void ) { return CLASS_NONE; }
 
public:
	void Spawn( void );
	void Precache( void );
	void FlyThink( void );
	void HookedThink( void );
	void HookTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CGrapplingHook *HookCreate( const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner = NULL );
	virtual bool ShouldDraw(void) { return false; }

protected:
 
	DECLARE_DATADESC();
 
private:

	CHandle<CWeaponGrapple>		m_hOwner;
	CHandle<CBasePlayer>		m_hPlayer;

	Vector grapple_dir;
	float dist;
};
 
#endif
 
//-----------------------------------------------------------------------------
// CWeaponGrapple
//-----------------------------------------------------------------------------
 
#ifdef CLIENT_DLL
#define CWeaponGrapple C_WeaponGrapple
#endif
 
class CWeaponGrapple : public CBaseHL2MPCombatWeapon {
	DECLARE_CLASS( CWeaponGrapple, CBaseHL2MPCombatWeapon );
public:
 
	CWeaponGrapple( void );
 
	virtual void	Precache( void );
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual bool	Deploy( void );
	virtual bool	Reload( void );
	virtual void	ItemPostFrame( void );
	virtual bool	SendWeaponAnim( int iActivity );
	void			NotifyHookDied( void );
	bool			HasAnyAmmo( void );
 
	CBaseEntity		*GetHook( void ) { return m_hHook; }

#ifndef CLIENT_DLL
	virtual void DisplayUsageHudHint(void);
#endif
 
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
 
private:

	void	FireHook( void );
	bool exploder;
 
	DECLARE_ACTTABLE();
 
private:

	CNetworkVar( bool,	m_bMustReload );
 
	CNetworkHandle( CBaseEntity, m_hHook );
 
	CWeaponGrapple( const CWeaponGrapple & );
};
 
 
 
#endif // WEAPON_GRAPPLE_H
