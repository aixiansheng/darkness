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

#include "convar.h"

#include "c_hl2mp_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_POINTS 0

//-----------------------------------------------------------------------------
// Purpose: Points panel
//-----------------------------------------------------------------------------
class CHudPoints : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudPoints, CHudNumericDisplay );

public:
	CHudPoints( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	void MsgFunc_Points( bf_read &msg );

private:
	int		m_iPoints;

protected:
	void ApplySchemeSettings(vgui::IScheme *pScheme);
};	

DECLARE_HUDELEMENT( CHudPoints );
DECLARE_HUD_MESSAGE( CHudPoints, Points );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudPoints::CHudPoints( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudPoints")
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudPoints::Init()
{
	HOOK_HUD_MESSAGE( CHudPoints, Points );
	Reset();
}

void CHudPoints::ApplySchemeSettings(vgui::IScheme *pScheme) {
	BaseClass::ApplySchemeSettings(pScheme);
	m_hTextFont = pScheme->GetFont("Default", true);
	m_hNumberFont = pScheme->GetFont("HudHintTextLarge", true);
}

void CHudPoints::Reset() {
	SetLabelText(L"POINTS");
	SetDisplayValue(m_iPoints);
}

void CHudPoints::VidInit() {
	Reset();
}

void CHudPoints::MsgFunc_Points( bf_read &msg ) {
	m_iPoints = msg.ReadByte();
	SetDisplayValue(m_iPoints);
}
