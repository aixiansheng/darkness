#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "vgui/IVGui.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
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

#define STATUS_FORMAT L"Health:%d/%d Armor:%d"

//-----------------------------------------------------------------------------
// Purpose: Item panel
//-----------------------------------------------------------------------------
class CHudItemStatus : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudItemStatus, EditablePanel );

public:
	CHudItemStatus( const char *pElementName );
	~CHudItemStatus(void);
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void ApplySchemeSettings(IScheme *scheme);
	void MsgFunc_ItemInfo( bf_read &msg );

	void ShowItemInfo(int health, int maxhealth, int armor);

private:
	Label *healthLabel;
	Label *armorLabel;

	Color normal;
	Color red;
};	

DECLARE_HUDELEMENT( CHudItemStatus );
DECLARE_HUD_MESSAGE( CHudItemStatus, ItemInfo );

CHudItemStatus::CHudItemStatus( const char *pElementName ) : 
	CHudElement( pElementName ), BaseClass(NULL, "HudItemStatus")
{
	Panel *pParent;
	IScheme* pScheme;
	HFont hFont;

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
	
	pParent = g_pClientMode->GetViewport();
	
	SetParent(pParent);
	//SetVisible(false);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	SetScheme(scheme()->LoadSchemeFromFile("resource/DarknessScheme.res", "DarknessScheme"));

	healthLabel = new Label(pParent, "HealthLabel", "health");
	armorLabel = new Label(pParent, "ArmorLabel", "armor");

	pScheme = vgui::scheme()->GetIScheme(GetScheme());
	
	if (pScheme) {
		hFont = pScheme->GetFont( "HudHintTextVeryLarge" );
		if (hFont) {
			if (healthLabel) {
				healthLabel->SetFont(hFont);
				healthLabel->SetVisible(false);
			}

			if (armorLabel) {
				armorLabel->SetFont(hFont);
				armorLabel->SetVisible(false);
			}
		}

		normal = GetSchemeColor("CHudItemStatus.FgColor", pScheme);
		red = Color(255, 0, 0, 255);
	}
}

CHudItemStatus::~CHudItemStatus(void) {
	if (healthLabel)
		delete healthLabel;
	
	if (armorLabel)
		delete armorLabel;
}

void CHudItemStatus::ApplySchemeSettings(IScheme *scheme) {
	BaseClass::ApplySchemeSettings(scheme);
	SetFgColor(normal);
}

void CHudItemStatus::Init() {
	HOOK_HUD_MESSAGE(CHudItemStatus, ItemInfo);

	Reset();
}

void CHudItemStatus::ShowItemInfo(int health, int maxhealth, int armor) {
	C_HL2MP_Player *local;
	char buf[64];
	float healthPercent;

	local = ToHL2MPPlayer(C_BasePlayer::GetLocalPlayer());
	if (!local)
		return;

	if (local->m_iClassNumber != CLASS_ENGINEER_IDX &&
		local->m_iClassNumber != CLASS_BREEDER_IDX)
		return;

	healthPercent = ((float)health / (float)maxhealth) * 100.0f;

	Q_snprintf(buf, sizeof(buf), "HEALTH: %d%% [%d/%d]", (int)healthPercent, health, maxhealth);
	if (healthLabel) {
		if (healthPercent < 50) {
			healthLabel->SetFgColor(red);
		} else {
			healthLabel->SetFgColor(normal);
		}

		healthLabel->SetText(buf);
		healthLabel->SetVisible(true);
	}

	if (armor > -1) {
		Q_snprintf(buf, sizeof(buf), "ARMOR: %d", armor);
		if (armorLabel) {
			armorLabel->SetVisible(true);
			armorLabel->SetText(buf);
		}
	} else {
		if (armorLabel) {
			armorLabel->SetVisible(false);
		}
	}

}

void CHudItemStatus::Reset() {
	SetVisible(false);
}

void CHudItemStatus::VidInit() {
	Reset();
}

void CHudItemStatus::MsgFunc_ItemInfo(bf_read &msg) {
	int armor, health, maxHealth;

	armor = msg.ReadShort();
	health = msg.ReadShort();
	maxHealth = msg.ReadShort();

	if (health > -1 || armor > -1) {
		ShowItemInfo(health, maxHealth, armor);
	} else {
		if (healthLabel)
			healthLabel->SetVisible(false);
		if (armorLabel)
			armorLabel->SetVisible(false);
	}
}
