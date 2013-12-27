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

#include "ConVar.h"

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
class CHudItemStatus : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudItemStatus, CHudNumericDisplay );

public:
	CHudItemStatus( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void OnTick();
	void UpdateLabel();
	void MsgFunc_ItemInfo( bf_read &msg );

private:
	// old variables
	int		armor;
	int		health;
};	

DECLARE_HUDELEMENT( CHudItemStatus );
DECLARE_HUD_MESSAGE( CHudItemStatus, ItemInfo );

CHudItemStatus::CHudItemStatus( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudItemStatus") {
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudItemStatus::Init() {
	HOOK_HUD_MESSAGE( CHudItemStatus, ItemInfo );

	vgui::ivgui()->AddTickSignal(GetVPanel(), 1000);

	Reset();
}

void CHudItemStatus::Reset() {
	armor = -1;
	health = -1;

	SetShouldDisplayValue(false);
	SetVisible(false);
}

#define BUFSIZE 64
void CHudItemStatus::UpdateLabel(void) {
	int teamNum;
	int classNum;
	int health_disp;
	int armor_disp;
	wchar_t buf[BUFSIZE] = { (wchar_t)0 };
	C_HL2MP_Player *local;

	if (armor == -1 && health == -1) {
		SetVisible(false);
		return;
	}

	local = ToHL2MPPlayer(C_BasePlayer::GetLocalPlayer());
	if (!local)
		return;

	teamNum = local->GetTeamNumber();
	classNum = local->m_iClassNumber;

	if (armor >= 0 || health >= 0) {
		if (armor < 0)
			armor_disp = 0;
		else
			armor_disp = armor;

		if (health < 0)
			health_disp = 0;
		else
			health_disp = health;

		if (teamNum == TEAM_SPIDERS && classNum == CLASS_BREEDER_IDX) {
			_snwprintf(buf, BUFSIZE, STATUS_FORMAT, health_disp, armor_disp);
			SetLabelText(buf);
			SetVisible(true);
			return;

		} else if (teamNum == TEAM_HUMANS && classNum == CLASS_ENGINEER_IDX) {
			_snwprintf(buf, BUFSIZE, STATUS_FORMAT, health_disp, armor_disp);
			SetLabelText(buf);
			SetVisible(true);
			return;
		}
	}

	SetVisible(false);

}

void CHudItemStatus::VidInit() {
	Reset();
}

void CHudItemStatus::OnThink() {
	UpdateLabel();
}

void CHudItemStatus::OnTick(void) {
	UpdateLabel();
}

void CHudItemStatus::MsgFunc_ItemInfo( bf_read &msg ) {
	armor = msg.ReadShort();
	health = msg.ReadShort();
	UpdateLabel();
}
