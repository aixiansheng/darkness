//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_hl2mp_player.h"
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "hl2mp_gamerules.h"
#include "c_hl2mp_player.h"
#include "class_info.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYER_HINT_DISTANCE	150
#define PLAYER_HINT_DISTANCE_SQ	(PLAYER_HINT_DISTANCE*PLAYER_HINT_DISTANCE)

static ConVar hud_centerid( "hud_centerid", "1" );
static ConVar hud_showtargetid( "hud_showtargetid", "1" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTargetID : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CTargetID, vgui::Panel );

public:
	CTargetID( const char *pElementName );
	void Init( void );
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint( void );
	void VidInit( void );

private:
	Color			GetColorForTargetTeam( int iTeamNumber );

	vgui::HFont		m_hFont;
	int				m_iLastEntIndex;
	float			m_flLastChangeTime;
};

DECLARE_HUDELEMENT( CTargetID );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTargetID::CTargetID( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "TargetID" )
{
	IScheme* pScheme;
	HFont hFont;

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	//m_hFont = g_hFontTrebuchet24;
	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	SetHiddenBits(HIDEHUD_MISCSTATUS);

	SetScheme(scheme()->LoadSchemeFromFile("resource/DarknessScheme.res", "DarknessScheme"));

	pScheme = vgui::scheme()->GetIScheme(GetScheme());
	
	if (pScheme) {
		m_hFont = pScheme->GetFont("HudHintTextVeryLarge");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void CTargetID::Init( void )
{
};

void CTargetID::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CTargetID::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

Color CTargetID::GetColorForTargetTeam( int iTeamNumber )
{
	return GameResources()->GetTeamColor( iTeamNumber );
} 

//-----------------------------------------------------------------------------
// Purpose: Draw function for the element
//-----------------------------------------------------------------------------
void CTargetID::Paint()
{
#define MAX_ID_STRING 256
	Color c;
	IScheme *s;
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();

	if (!pPlayer)
		return;

	s = scheme()->GetIScheme(GetScheme());
	c = GetSchemeColor("CHudItemStatus.FgColor", s);

	int iEntIndex = pPlayer->GetIDTarget();

	if (!iEntIndex) {
		if (m_flLastChangeTime && (gpGlobals->curtime > (m_flLastChangeTime + 0.5)) ) {
			m_flLastChangeTime = 0;
			m_iLastEntIndex = 0;
		} else {
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	// Is this an entindex sent by the server?
	if ( iEntIndex )
	{
		C_BasePlayer *pPlayer = static_cast<C_BasePlayer*>(cl_entitylist->GetEnt( iEntIndex ));
		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		C_HL2MP_Player *p = ToHL2MPPlayer(pPlayer);

		if (!p || !pPlayer || !pLocalPlayer)
			return;

		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		if (IsPlayerIndex(iEntIndex)) {

			if (p->GetTeamNumber() != pLocalPlayer->GetTeamNumber()) {
				return;
			}

			if (p->m_iClassNumber == CLASS_GUARDIAN_IDX &&
				p->IsStopped() == true) {
					return;
			}

			g_pVGuiLocalize->ConvertANSIToUnicode(pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName));

			vgui::surface()->DrawSetTextFont(m_hFont);
			vgui::surface()->DrawSetTextColor(c);
			vgui::surface()->DrawPrintText(wszPlayerName, wcslen(wszPlayerName));

		}

	}
}
