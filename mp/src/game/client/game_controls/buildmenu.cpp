//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include "buildmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <filesystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

#include "cdll_util.h"
#include "IGameUIFuncs.h" // for key bindings
#ifndef _XBOX
extern IGameUIFuncs *gameuifuncs; // for key binding details
#endif
#include <game/client/iviewport.h>

#if defined( TF_CLIENT_DLL )
#include "item_inventory.h"
#endif // TF_CLIENT_DLL

#include "hl2mp_gamerules.h"
#include "c_hl2mp_player.h"
#include "item_info.h"
#include "c_team.h"
#include <stdlib.h> // MAX_PATH define

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBuildMenu::CBuildMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_BUILD)
{
	m_pViewPort = pViewPort;
	m_iTeam = 0;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	// info window about this class
	m_pPanel = new EditablePanel( this, "BuildMeniInfo" );

	LoadControlSettings("Resource/UI/BuildMenu.res");
	InvalidateLayout();

}

CBuildMenu::CBuildMenu(IViewPort *pViewPort, const char *panelName) : Frame(NULL, panelName)
{
	m_pViewPort = pViewPort;
	m_iTeam = 0;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	// info window about this class
	m_pPanel = new EditablePanel( this, "BuildMenuInfo" );

	// Inheriting classes are responsible for calling LoadControlSettings()!

}

CBuildMenu::~CBuildMenu() {}

//-----------------------------------------------------------------------------
// Purpose: Called when the user picks a class
//-----------------------------------------------------------------------------
void CBuildMenu::OnCommand( const char *command )
{
	if (Q_stricmp( command, "vguicancel" )) {
		engine->ClientCmd(const_cast<char *>(command));
	}

	Close();
	gViewPortInterface->ShowBackGround(false);

	BaseClass::OnCommand( command );
}


void CBuildMenu::Update(void) {
	Label *label;
	int points, team;
	char pointstr[64] = {0};
	C_HL2MP_Player *player;
	C_Team *cteam;

	player = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (!player) {
		Close();
		gViewPortInterface->ShowBackGround(false);
		return;
	}

	team = player->GetTeamNumber();
	if (team != TEAM_HUMANS && team != TEAM_SPIDERS) {
		Close();
		gViewPortInterface->ShowBackGround(false);
		return;
	}

	cteam = player->GetTeam();
	if (cteam == NULL) {
		Close();
		gViewPortInterface->ShowBackGround(false);
		return;
	}

	label = dynamic_cast<Label *>(FindChildByName("pointsArea"));
	if (label) {
		points = cteam->GetAssetPoints();
		Q_snprintf(pointstr, sizeof(pointstr), "Build Points: %d", points);
		label->SetText((const char *)pointstr);
	}

	team = player->GetTeamNumber();
	ClearItemButtons();
	if (team == TEAM_SPIDERS) {
		ShowItemButton(0, ITEM_EGG_IDX);
		ShowItemButton(1, ITEM_HEALER_IDX);
		ShowItemButton(2, ITEM_SPIKER_IDX);
		ShowItemButton(3, ITEM_OBSTACLE_IDX);
		ShowItemButton(4, ITEM_GASSER_IDX);
	} else if (team == TEAM_HUMANS) {
		ShowItemButton(0, ITEM_TELEPORTER_IDX);
		ShowItemButton(1, ITEM_AMMO_CRATE_IDX);
		ShowItemButton(2, ITEM_MEDIPAD_IDX);
		ShowItemButton(3, ITEM_MINE_IDX);
		ShowItemButton(4, ITEM_SMG_TURRET_IDX);
		ShowItemButton(5, ITEM_DETECTOR_IDX);
		ShowItemButton(6, ITEM_MSL_TURRET_IDX);
	}

}

//-----------------------------------------------------------------------------
// Purpose: shows the class menu
//-----------------------------------------------------------------------------
void CBuildMenu::ShowPanel(bool bShow) {

	if ( BaseClass::IsVisible() == bShow )
		return;
	
	if (bShow) {
		Update();
	}

	if ( bShow ) {
		Activate();
		SetMouseInputEnabled( true );
	} else {
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
	
	m_pViewPort->ShowBackGround( bShow );

}


#define BUTTON_STR		"builditem"
#define BUTTON_FMT_STR	BUTTON_STR "%d"

// see ClassMenu.res for max...
#define MAX_BUTTONS 9

void CBuildMenu::ClearItemButtons(void) {
	int i;
	char buf[sizeof(BUTTON_STR) + 1];
	Button *b;
	
	for (i = 0; i < MAX_BUTTONS; i++) {
		Q_snprintf(buf, sizeof(buf), BUTTON_FMT_STR, i);
		b = dynamic_cast<Button *>(FindChildByName((const char *)buf));
		if (b) {
			b->SetText("None");
			b->SetVisible(false);
		}
	}
}

void CBuildMenu::ShowItemButton(int buttonNum, int itemNum) {
	CHL2MP_Player *p;
	char buttonName[32];
	char commandStr[32];
	char buttonText[32];
	const char *name;
	int cost;
	int available_points;
	Button *b;

	available_points = 10;

	p = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (p)
		available_points = p->GetTeam()->GetAssetPoints();

	name = dk_items[itemNum].display_name;
	cost = dk_items[itemNum].value;

	Q_snprintf(buttonName, sizeof(buttonName), "builditem%d", buttonNum);
	Q_snprintf(commandStr, sizeof(commandStr), "builditem %d", itemNum);
	Q_snprintf(buttonText, sizeof(buttonText), "&%d. %s [%d points]", buttonNum + 1, name, cost);

	b = dynamic_cast<Button *>(FindChildByName((const char *)buttonName));
	if (b) {
		b->SetText(buttonText);
		b->SetVisible(true);
		b->SetCommand(commandStr);
		b->SetButtonBorderEnabled(false);
		b->SetPaintBorderEnabled(false);
		b->SetEnabled(true);

		if (available_points < cost) {
			b->SetEnabled(false);
		}
	}
}

void CBuildMenu::SetData(KeyValues *data) {
	return;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CBuildMenu::SetLabelText(const char *textEntryName, const char *text) {
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry) {
		entry->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the visibility of a button by name
//-----------------------------------------------------------------------------
void CBuildMenu::SetVisibleButton(const char *textEntryName, bool state) {
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));
	if (entry) {
		entry->SetVisible(state);
	}
}

