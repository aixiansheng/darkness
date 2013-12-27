#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "ConVar.h"

#include "c_hl2mp_player.h"
#include "c_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_POINTS -1

class C_Team;

//-----------------------------------------------------------------------------
// Purpose: Points panel
//-----------------------------------------------------------------------------
class CHudAssets : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudAssets, CHudNumericDisplay );

public:
	CHudAssets( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	void MsgFunc_Damage( bf_read &msg );

private:
	// old variables
	int		m_iPoints;
	
	int		m_bitsDamage;
};	

DECLARE_HUDELEMENT( CHudAssets );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAssets::CHudAssets( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudAssets")
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAssets::Init()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAssets::Reset()
{
	m_iPoints		= INIT_POINTS;
	m_bitsDamage	= 0;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_ASSET_POINTS");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"BUILD POINTS");
	}
	SetDisplayValue(m_iPoints);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAssets::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAssets::OnThink()
{
	int newPoints = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	C_Team *team;

	if ( local ) {
		// Never below zero
		team = local->GetTeam();
		if (team) {
			newPoints = max( team->GetAssetPoints(), 0);
		}
	}

	// Only update the fade if we've changed health
	if ( newPoints == m_iPoints ) {
		return;
	}

	m_iPoints = newPoints;

	SetDisplayValue(m_iPoints);
}
