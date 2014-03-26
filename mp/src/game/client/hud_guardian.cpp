#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "vgui/IVGui.h"

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
#include "c_team.h"
#include "class_info.h"
#include "item_info.h"
#include "hl2mp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_Team;

#define STATUS_FORMAT L"Health: %d    Armor:%d"

//-----------------------------------------------------------------------------
// Purpose: Item panel
//-----------------------------------------------------------------------------
class CHudGuardian : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudGuardian, CHudNumericDisplay );

public:
	CHudGuardian( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	void UpdateLabel();
	void MsgFunc_GuardianHide( bf_read &msg );

private:
	bool show;
};	

DECLARE_HUDELEMENT( CHudGuardian );
DECLARE_HUD_MESSAGE( CHudGuardian, GuardianHide);

CHudGuardian::CHudGuardian( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudGuardian") {
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudGuardian::Init() {
	HOOK_HUD_MESSAGE( CHudGuardian, GuardianHide );
	Reset();
}

void CHudGuardian::Reset() {
	SetLabelText(L"Invisible");
	show = false;
	SetVisible(false);
	SetShouldDisplayValue(false);
}

void CHudGuardian::UpdateLabel(void) {
	if (show) {
		SetVisible(true);
		SetAlpha(255);
	} else {
		SetVisible(false);
		SetAlpha(0);
	}
}

void CHudGuardian::VidInit() {
	Reset();
}

void CHudGuardian::MsgFunc_GuardianHide( bf_read &msg ) {
	show = (bool)msg.ReadByte();
	UpdateLabel();
}
