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

//-----------------------------------------------------------------------------
// Purpose: Item panel
//-----------------------------------------------------------------------------
class CHudItemSelect : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudItemSelect, CHudNumericDisplay );

public:
	CHudItemSelect( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void OnTick();
	void UpdateLabel();
	void MsgFunc_ItemChange( bf_read &msg );

private:
	// old variables
	int		m_iItem;
	int		m_iTeam;
	int		m_iClass;
};	

DECLARE_HUDELEMENT( CHudItemSelect );
DECLARE_HUD_MESSAGE( CHudItemSelect, ItemChange );

CHudItemSelect::CHudItemSelect( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudItemSelect") {
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

void CHudItemSelect::Init() {
	HOOK_HUD_MESSAGE( CHudItemSelect, ItemChange );

	vgui::ivgui()->AddTickSignal(GetVPanel(), 500);

	Reset();
}

void CHudItemSelect::Reset() {
	m_iTeam = -1;
	m_iItem = -1;
	m_iClass = -1;

	SetShouldDisplayValue(false);
	SetVisible(false);
}

void CHudItemSelect::UpdateLabel(void) {
	C_HL2MP_Player *local;

	local = ToHL2MPPlayer(C_BasePlayer::GetLocalPlayer());
	if (!local)
		return;

	m_iTeam = local->GetTeamNumber();
	m_iClass = local->m_iClassNumber;
	m_iItem = local->m_iSelectedItem;

	if (local->IsAlive() && ((m_iTeam == TEAM_SPIDERS || m_iTeam == TEAM_HUMANS) &&
		(m_iClass == CLASS_ENGINEER_IDX || m_iClass == CLASS_BREEDER_IDX))) {

			if (m_iTeam == TEAM_HUMANS && m_iItem >= 0 && m_iItem < NUM_HUMAN_ITEMS) {
				SetLabelText(human_items[m_iItem].display_name);
			} else if (m_iTeam == TEAM_SPIDERS && m_iItem >= 0 && m_iItem < NUM_SPIDER_ITEMS) {
				SetLabelText(spider_items[m_iItem].display_name);
			}
			
			SetVisible(true);

	} else {
		SetVisible(false);
	}

}

void CHudItemSelect::VidInit() {
	Reset();
}

void CHudItemSelect::OnThink() {
	UpdateLabel();
}

void CHudItemSelect::OnTick(void) {
	UpdateLabel();
}

void CHudItemSelect::MsgFunc_ItemChange( bf_read &msg ) {
	UpdateLabel();
}
