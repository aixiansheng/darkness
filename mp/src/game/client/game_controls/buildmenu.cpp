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
	if (team == TEAM_SPIDERS) {
		ShowItems(spider_items, NUM_SPIDER_ITEMS);
	} else if (team == TEAM_HUMANS) {
		ShowItems(human_items, NUM_HUMAN_ITEMS);
	} else {
		ShowItems(NULL, 0);
		return;
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

// see BuildMenu.res for max...
#define MAX_BUTTONS 9


void CBuildMenu::ShowItems(item_info_t *infos, int num) {
	int i;
	char buf[sizeof(BUTTON_STR) + 1];
	char dispBuf[32];
	Button *b;
	
	if (infos == NULL || num == 0) {
		for (i = 0; i < MAX_BUTTONS; i++) {
			Q_snprintf(buf, sizeof(buf), BUTTON_FMT_STR, i);
			b = dynamic_cast<Button *>(FindChildByName((const char *)buf));
			if (b) {
				b->SetText("None");
				b->SetVisible(false);
			}
		}
	}

	// num > 9 = bad... so don't do it :D
	for (i = 0; i < num; i++) {
		Q_snprintf(buf, sizeof(buf), BUTTON_FMT_STR, i);
		b = dynamic_cast<Button *>(FindChildByName((const char *)buf));
		if (b) {
			Q_snprintf(dispBuf, sizeof(dispBuf), "&%d. %ls  [%d points]", i + 1, infos[i].display_name, infos[i].value);
			b->SetText(dispBuf);
			b->SetVisible(true);
			b->SetButtonBorderEnabled(false);
			b->SetPaintBorderEnabled(false);
		}
	}

	if (i < MAX_BUTTONS - 1) {
		for (; i < MAX_BUTTONS; i++) {
			Q_snprintf(buf, sizeof(buf), BUTTON_FMT_STR, i);
			b = dynamic_cast<Button *>(FindChildByName((const char *)buf));
			if (b) {
				b->SetText("None");
				b->SetVisible(false);
			}
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

