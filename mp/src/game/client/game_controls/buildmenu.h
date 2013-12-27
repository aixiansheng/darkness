//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BUILDMENU_H
#define BUILDMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <utlvector.h>
#include <vgui/ILocalize.h>
#include <vgui/KeyCode.h>
#include <game/client/iviewport.h>

#include "mouseoverpanelbutton.h"
#include "item_info.h"
#include "hud_macros.h"

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// Purpose: Draws the class menu
//-----------------------------------------------------------------------------
class CBuildMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CBuildMenu, vgui::Frame );

public:
	CBuildMenu(IViewPort *pViewPort);
	CBuildMenu(IViewPort *pViewPort, const char *panelName );
	virtual ~CBuildMenu();

	virtual const char *GetName( void ) { return PANEL_BUILD; }
	virtual void SetData(KeyValues *data);
	virtual void Reset() {};
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

protected:

	// helper functions
	void SetLabelText(const char *textEntryName, const char *text);
	void SetVisibleButton(const char *textEntryName, bool state);

	// command callbacks
	void OnCommand( const char *command );

	IViewPort *m_pViewPort;
	int m_iTeam;
	vgui::EditablePanel *m_pPanel;

	CUtlVector< MouseOverPanelButton * > m_mouseoverButtons;

	void ShowItems(item_info_t *infos, int num);
};


#endif // CLASSMENU_H
