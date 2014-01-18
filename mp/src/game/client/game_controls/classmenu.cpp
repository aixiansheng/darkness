//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include "classmenu.h"

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
#include "class_info.h"
#include <stdlib.h> // MAX_PATH define

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#ifdef TF_CLIENT_DLL
#define HUD_CLASSAUTOKILL_FLAGS		( FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_USERINFO )
#else
#define HUD_CLASSAUTOKILL_FLAGS		( FCVAR_CLIENTDLL | FCVAR_ARCHIVE )
#endif // !TF_CLIENT_DLL

ConVar hud_classautokill( "hud_classautokill", "1", HUD_CLASSAUTOKILL_FLAGS, "Automatically kill player after choosing a new playerclass." );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClassMenu::CClassMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_CLASS)
{
	m_pViewPort = pViewPort;
	m_iScoreBoardKey = BUTTON_CODE_INVALID; // this is looked up in Activate()
	m_iTeam = 0;
	m_iPoints = 0;

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
	m_pPanel = new EditablePanel( this, "ClassInfo" );

	LoadControlSettings("Resource/UI/ClassMenu.res");
	InvalidateLayout();
}

CClassMenu::CClassMenu(IViewPort *pViewPort, const char *panelName) : Frame(NULL, panelName)
{
	m_pViewPort = pViewPort;
	m_iScoreBoardKey = BUTTON_CODE_INVALID; // this is looked up in Activate()
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
	m_pPanel = new EditablePanel( this, "ClassInfo" );

	// Inheriting classes are responsible for calling LoadControlSettings()!
}

CClassMenu::~CClassMenu() {}

//-----------------------------------------------------------------------------
// Purpose: Called when the user picks a class
//-----------------------------------------------------------------------------
void CClassMenu::OnCommand( const char *command )
{
	if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( const_cast<char *>( command ) );

#if !defined( CSTRIKE_DLL ) && !defined( TF_CLIENT_DLL )
		// They entered a command to change their class, kill them so they spawn with 
		// the new class right away

		//if ( hud_classautokill.GetBool() ) {
  //          engine->ClientCmd( "kill" );
		//}
#endif // !CSTRIKE_DLL && !TF_CLIENT_DLL
	}

	Close();

	gViewPortInterface->ShowBackGround( false );

	BaseClass::OnCommand( command );
	engine->ClientCmd(const_cast<char *>(command));
}


void CClassMenu::Update(void) {
	Label *label;
	int points;
	char pointstr[64] = {0};
	C_HL2MP_Player *player;

	player = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (player == NULL) {
		player = C_HL2MP_Player::GetLocalHL2MPPlayer();
	}

	if (!player) {
		ShowClasses(NULL, 0);
		return;
	}

	label = dynamic_cast<Label *>(FindChildByName("pointsArea"));
	if (label) {
		points = player->GetPlayerPoints();
		Q_snprintf(pointstr, sizeof(pointstr), "You have %d points", points);
		label->SetText((const char *)pointstr);
	}

	m_iTeam = player->GetTeamNumber();
	if (m_iTeam == TEAM_SPIDERS) {
		ShowClasses(dk_spider_classes, NUM_SPIDER_CLASSES);
	} else if (m_iTeam == TEAM_HUMANS) {
		ShowClasses(dk_human_classes, NUM_HUMAN_CLASSES);
	} else {
		ShowClasses(NULL, 0);
		return;
	}

}

//-----------------------------------------------------------------------------
// Purpose: shows the class menu
//-----------------------------------------------------------------------------
void CClassMenu::ShowPanel(bool bShow) {
	Label *label;
	int points;
	char pointstr[64] = {0};
	C_HL2MP_Player *player;

	player = C_HL2MP_Player::GetLocalHL2MPPlayer();

	if ( BaseClass::IsVisible() == bShow )
		return;

	if (!player) {
		ShowClasses(NULL, 0);
		return;
	}

	label = dynamic_cast<Label *>(FindChildByName("pointsArea"));
	if (label) {
		points = player->GetPlayerPoints();
		Q_snprintf(pointstr, sizeof(pointstr), "You have %d points", points);
		label->SetText((const char *)pointstr);
	}


	m_iTeam = player->GetTeamNumber();
	if (m_iTeam == TEAM_SPIDERS) {
		ShowClasses(dk_spider_classes, NUM_SPIDER_CLASSES);
	} else if (m_iTeam == TEAM_HUMANS) {
		ShowClasses(dk_human_classes, NUM_HUMAN_CLASSES);
	} else {
		ShowClasses(NULL, 0);
		return;
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

#define BUTTON_STR		"switchclass"
#define BUTTON_FMT_STR	BUTTON_STR "%d"

// see ClassMenu.res for max...
#define MAX_BUTTONS 9


void CClassMenu::ShowClasses(class_info_t *infos, int num) {
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
			Q_snprintf(dispBuf, sizeof(dispBuf), "&%d. %s  [%d points]", i + 1, infos[i].name, infos[i].cost);
			b->SetText(dispBuf);
			b->SetVisible(true);
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

void CClassMenu::SetData(KeyValues *data) {
	return;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CClassMenu::SetLabelText(const char *textEntryName, const char *text) {
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry) {
		entry->SetText(text);
	}
}

void CClassMenu::OnThink(void) {
	int team;
	int points;
	C_HL2MP_Player *player;

	player = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (player) {
		team = player->GetTeamNumber();
		points = player->GetPlayerPoints();
		if (team != m_iTeam || points != m_iPoints) {
			m_iTeam = team;
			m_iPoints = points;
			if (IsVisible()) {
				Update();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the visibility of a button by name
//-----------------------------------------------------------------------------
void CClassMenu::SetVisibleButton(const char *textEntryName, bool state) {
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));
	if (entry) {
		entry->SetVisible(state);
	}
}

void CClassMenu::OnKeyCodePressed(KeyCode code)
{
	int nDir = 0;

	switch ( code )
	{
	case KEY_XBUTTON_UP:
	case KEY_XSTICK1_UP:
	case KEY_XSTICK2_UP:
	case KEY_UP:
	case KEY_XBUTTON_LEFT:
	case KEY_XSTICK1_LEFT:
	case KEY_XSTICK2_LEFT:
	case KEY_LEFT:
		nDir = -1;
		break;

	case KEY_XBUTTON_DOWN:
	case KEY_XSTICK1_DOWN:
	case KEY_XSTICK2_DOWN:
	case KEY_DOWN:
	case KEY_XBUTTON_RIGHT:
	case KEY_XSTICK1_RIGHT:
	case KEY_XSTICK2_RIGHT:
	case KEY_RIGHT:
		nDir = 1;
		break;
	}

	if ( m_iScoreBoardKey != BUTTON_CODE_INVALID && m_iScoreBoardKey == code )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );
		gViewPortInterface->PostMessageToPanel( PANEL_SCOREBOARD, new KeyValues( "PollHideCode", "code", code ) );
	}
	else if ( nDir != 0 )
	{
		CUtlSortVector< SortedPanel_t, CSortedPanelYLess > vecSortedButtons;
		VguiPanelGetSortedChildButtonList( this, (void*)&vecSortedButtons, "&", 0 );

		int nNewArmed = VguiPanelNavigateSortedChildButtonList( (void*)&vecSortedButtons, nDir );

		if ( nNewArmed != -1 )
		{
			// Handled!
			if ( nNewArmed < m_mouseoverButtons.Count() )
			{
				m_mouseoverButtons[ nNewArmed ]->OnCursorEntered();
			}
			return;
		}
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

