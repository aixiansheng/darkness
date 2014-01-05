//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Label.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "text_message.h"
#include "c_baseplayer.h"
#include "IGameUIFuncs.h"
#include "inputsystem/iinputsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays hints across the center of the screen
//-----------------------------------------------------------------------------
class CHudHintDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHintDisplay, vgui::Panel );

public:
	CHudHintDisplay( const char *pElementName );

	void Init();
	void Reset();
	void MsgFunc_HintText( bf_read &msg );
	void FireGameEvent( IGameEvent * event);

	bool SetHintText( wchar_t *text );
	void LocalizeAndDisplay( const char *pszHudTxtMsg, const char *szRawString );

	virtual void PerformLayout();

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

protected:
	vgui::HFont m_hFont;
	Color		m_bgColor;
	vgui::Label *m_pLabel;
	CUtlVector<vgui::Label *> m_Labels;
	CPanelAnimationVarAliasType( int, m_iTextX, "text_xpos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextY, "text_ypos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCenterX, "center_x", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCenterY, "center_y", "0", "proportional_int" );

	bool		m_bLastLabelUpdateHack;
	CPanelAnimationVar( float, m_flLabelSizePercentage, "HintSize", "0" );
};

DECLARE_HUDELEMENT( CHudHintDisplay );
DECLARE_HUD_MESSAGE( CHudHintDisplay, HintText );

#define MAX_HINT_STRINGS 5


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHintDisplay::CHudHintDisplay( const char *pElementName ) : BaseClass(NULL, "HudHintDisplay"), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	m_pLabel = new vgui::Label( this, "HudHintDisplayLabel", "" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::Init()
{
	HOOK_HUD_MESSAGE( CHudHintDisplay, HintText );

	// listen for client side events
	ListenForGameEvent( "player_hintmessage" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::Reset()
{
	SetHintText( NULL );
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageHide" ); 
	m_bLastLabelUpdateHack = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( GetSchemeColor("HintMessageFg", pScheme) );
	m_hFont = pScheme->GetFont( "HudHintText", true );
	m_pLabel->SetBgColor( GetSchemeColor("HintMessageBg", pScheme) );
	m_pLabel->SetPaintBackgroundType( 2 );
	m_pLabel->SetSize( 0, GetTall() );		// Start tiny, it'll grow.
}

//-----------------------------------------------------------------------------
// Purpose: Sets the hint text, replacing variables as necessary
//-----------------------------------------------------------------------------
bool CHudHintDisplay::SetHintText( wchar_t *text )
{
	if ( text == NULL || text[0] == L'\0' )
	{
		return false;
	}

	// clear the existing text
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->MarkForDeletion();
	}
	m_Labels.RemoveAll();

	wchar_t *p = text;

	while ( p )
	{
		wchar_t *line = p;
		wchar_t *end = wcschr( p, L'\n' );
		int linelengthbytes = 0;
		if ( end )
		{
			//*end = 0;	//eek
			p = end+1;
			linelengthbytes = ( end - line ) * 2;
		}
		else
		{
			p = NULL;
		}		

		// replace any key references with bound keys
		wchar_t buf[512];
		UTIL_ReplaceKeyBindings( line, linelengthbytes, buf, sizeof( buf ) );

		// put it in a label
		vgui::Label *label = vgui::SETUP_PANEL(new vgui::Label(this, NULL, buf));
		label->SetFont( m_hFont );
		label->SetPaintBackgroundEnabled( false );
		label->SetPaintBorderEnabled( false );
		label->SizeToContents();
		label->SetContentAlignment( vgui::Label::a_west );
		label->SetFgColor( GetFgColor() );
		m_Labels.AddToTail( vgui::SETUP_PANEL(label) );
	}

	InvalidateLayout( true );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Resizes the label
//-----------------------------------------------------------------------------
void CHudHintDisplay::PerformLayout()
{
	BaseClass::PerformLayout();
	int i;

	int wide, tall;
	GetSize( wide, tall );

	// find the widest line
	int iDesiredLabelWide = 0;
	for ( i=0; i < m_Labels.Count(); ++i )
	{
		iDesiredLabelWide = MAX( iDesiredLabelWide, m_Labels[i]->GetWide() );
	}

	// find the total height
	int fontTall = vgui::surface()->GetFontTall( m_hFont );
	int labelTall = fontTall * m_Labels.Count();

	iDesiredLabelWide += m_iTextX*2;
	labelTall += m_iTextY*2;

	// Now clamp it to our animation size
	iDesiredLabelWide = (iDesiredLabelWide * m_flLabelSizePercentage);

	int x, y;
	if ( m_iCenterX < 0 )
	{
		x = 0;
	}
	else if ( m_iCenterX > 0 )
	{
		x = wide - iDesiredLabelWide;
	}
	else
	{
		x = (wide - iDesiredLabelWide) / 2;
	}

	if ( m_iCenterY > 0 )
	{
		y = 0;
	}
	else if ( m_iCenterY < 0 )
	{
		y = tall - labelTall;
	}
	else
	{
		y = (tall - labelTall) / 2;
	}

	x = MAX(x,0);
	y = MAX(y,0);

	iDesiredLabelWide = MIN(iDesiredLabelWide,wide);
	m_pLabel->SetBounds( x, y, iDesiredLabelWide, labelTall );

	// now lay out the sub-labels
	for ( i=0; i<m_Labels.Count(); ++i )
	{
		int xOffset = (wide - m_Labels[i]->GetWide()) * 0.5;
		m_Labels[i]->SetPos( xOffset, y + m_iTextY + i*fontTall );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudHintDisplay::OnThink()
{
	m_pLabel->SetFgColor(GetFgColor());
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->SetFgColor(GetFgColor());
	}

	// If our label size isn't at the extreme's, we're sliding open / closed
	// This is a hack to get around InvalideLayout() not getting called when
	// m_flLabelSizePercentage is changed via a HudAnimation.
	if ( ( m_flLabelSizePercentage != 0.0 && m_flLabelSizePercentage != 1.0 ) || m_bLastLabelUpdateHack )
	{
		m_bLastLabelUpdateHack = (m_flLabelSizePercentage != 0.0 && m_flLabelSizePercentage != 1.0);
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Activates the hint display
//-----------------------------------------------------------------------------
void CHudHintDisplay::MsgFunc_HintText( bf_read &msg )
{
	// Read the string(s)
	char szString[255];
	msg.ReadString( szString, sizeof(szString) );

	char *tmpStr = hudtextmessage->LookupString( szString, NULL );
	LocalizeAndDisplay( tmpStr, szString );
}

//-----------------------------------------------------------------------------
// Purpose: Activates the hint display upon receiving a hint
//-----------------------------------------------------------------------------
void CHudHintDisplay::FireGameEvent( IGameEvent * event)
{
	const char *hintmessage = event->GetString( "hintmessage" );
	char *tmpStr = hudtextmessage->LookupString( hintmessage, NULL );
	LocalizeAndDisplay( tmpStr, hintmessage );
}

extern ConVar sv_hudhint_sound;
ConVar cl_hudhint_sound( "cl_hudhint_sound", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Disable hudhint sounds." );

//-----------------------------------------------------------------------------
// Purpose: Localize, display, and animate the hud element
//-----------------------------------------------------------------------------
void CHudHintDisplay::LocalizeAndDisplay( const char *pszHudTxtMsg, const char *szRawString )
{
	static wchar_t szBuf[128];
	wchar_t *pszBuf;

	// init buffers & pointers
	szBuf[0] = 0;
	pszBuf = szBuf;

	// try to localize
	if ( pszHudTxtMsg )
	{
		pszBuf = g_pVGuiLocalize->Find( pszHudTxtMsg );
	}
	else
	{
		pszBuf = g_pVGuiLocalize->Find( szRawString );
	}

	if ( !pszBuf )
	{
		// use plain ASCII string 
		g_pVGuiLocalize->ConvertANSIToUnicode( szRawString, szBuf, sizeof(szBuf) );
		pszBuf = szBuf;
	}

	// make it visible
	if ( SetHintText( pszBuf ) )
	{
		SetVisible( true );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageShow" ); 

		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pLocalPlayer )
		{
#ifndef HL2MP
			if ( sv_hudhint_sound.GetBool() && cl_hudhint_sound.GetBool() )
			{
				pLocalPlayer->EmitSound( "Hud.Hint" );
			}
#endif // HL2MP

			if ( pLocalPlayer->Hints() )
			{
				pLocalPlayer->Hints()->PlayedAHint();
			}
		}
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageHide" ); 
	}
}




//-----------------------------------------------------------------------------
// Purpose: Displays small key-centric hints on the right hand side of the screen
//-----------------------------------------------------------------------------
class CHudHintKeyDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHintKeyDisplay, vgui::Panel );

public:
	CHudHintKeyDisplay( const char *pElementName );
	void Init();
	void Reset();
	void MsgFunc_KeyHintText( bf_read &msg );
	bool ShouldDraw();

	bool SetHintText( const char *text );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();
	void TokenizeLabels(wchar_t *ws);

private:
	CUtlVector<vgui::Label *> m_Labels;
	vgui::HFont m_hSmallFont, m_hLargeFont;
	int		m_iBaseY;

	CPanelAnimationVarAliasType( float, m_iTextX, "text_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextY, "text_ypos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextGapX, "text_xgap", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextGapY, "text_ygap", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iYOffset, "YOffset", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudHintKeyDisplay );
DECLARE_HUD_MESSAGE( CHudHintKeyDisplay, KeyHintText );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHintKeyDisplay::CHudHintKeyDisplay( const char *pElementName ) : BaseClass(NULL, "HudHintKeyDisplay"), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	SetAlpha( 0 );

	// not until we have other localizations
	// if (g_pVGuiLocalize->AddFile("resource/darkness_%language%.txt") == false) {
	if (g_pVGuiLocalize->AddFile("resource/darkness_english.txt") == false) {
		Warning("Failed to load darkness localization file\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintKeyDisplay::Init()
{
	HOOK_HUD_MESSAGE( CHudHintKeyDisplay, KeyHintText );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintKeyDisplay::Reset()
{
	SetHintText( NULL );
	SetAlpha( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintKeyDisplay::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	m_hSmallFont = pScheme->GetFont( "HudHintTextSmall", true );
	m_hLargeFont = pScheme->GetFont( "HudHintTextLarge", false );

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudHintKeyDisplay::ShouldDraw( void )
{
	return ( ( GetAlpha() > 0 ) && CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudHintKeyDisplay::OnThink()
{
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		if ( IsX360() && ( i & 1 ) == 0 )
		{
			// Don't change the fg color for buttons (even numbered labels)
			m_Labels[i]->SetAlpha( GetFgColor().a() );
		}
		else
		{
			m_Labels[i]->SetFgColor(GetFgColor());
		}
	}

	//int ox, oy;
	//GetPos(ox, oy);
	//SetPos( ox, m_iBaseY + m_iYOffset );
}

void CHudHintKeyDisplay::TokenizeLabels(wchar_t *ws) {

	wchar_t *p = ws;

	while ( p )
	{
		wchar_t *line = p;
		wchar_t *end = wcschr( p, L'\n' );
		int linelengthbytes = 0;
		if ( end )
		{
			//*end = 0;	//eek
			p = end+1;
			linelengthbytes = ( end - line ) * 2;
		}
		else
		{
			p = NULL;
		}		

		// replace any key references with bound keys
		wchar_t buf[512];
		UTIL_ReplaceKeyBindings( line, linelengthbytes, buf, sizeof( buf ) );

		// put it in a label
		vgui::Label *label = vgui::SETUP_PANEL(new vgui::Label(this, NULL, buf));
		label->SetFont( m_hLargeFont );
		label->SetPaintBackgroundEnabled( false );
		label->SetPaintBorderEnabled( false );
		label->SizeToContents();
		label->SetContentAlignment( vgui::Label::a_west );
		label->SetFgColor( GetFgColor() );
		m_Labels.AddToTail( vgui::SETUP_PANEL(label) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the hint text, replacing variables as necessary
//-----------------------------------------------------------------------------
bool CHudHintKeyDisplay::SetHintText( const char *text )
{
	if ( text == NULL || text[0] == L'\0' )
		return false;

	// clear the existing text
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->MarkForDeletion();
	}
	m_Labels.RemoveAll();

	// look up the text string
	wchar_t *ws = g_pVGuiLocalize->Find( text );

	wchar_t wszBuf[256];
	if ( !ws || wcslen(ws) <= 0)
	{
		Warning("Couldn't find localization for key %s\n", text);
		if (text[0] == '#')
		{
			// We don't want to display a localization placeholder, do we?
			return false;
		}
		// use plain ASCII string 
		g_pVGuiLocalize->ConvertANSIToUnicode(text, wszBuf, sizeof(wszBuf));
		ws = wszBuf;
	}

	TokenizeLabels(ws);

	if (strstr(text, "DK_Hint_Class")) {
		ws = g_pVGuiLocalize->Find("DK_Hint_General");
		if (ws && wcslen(ws) > 0) {
			TokenizeLabels(ws);
		}
	}

// Enable this small block of code to test formatting and layout of hint messages
// with varying numbers of lines
#define TEST_KEYHINT_DISPLAY 0
#if TEST_KEYHINT_DISPLAY

	// clear the existing text
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->MarkForDeletion();
	}
	m_Labels.RemoveAll();

	const char* sampleText[] = 
	{
		"This is a test",
		"of the hint system\nwith a multi-line hint",
		"that\ngoes\non\nfor",
		"several",
		"lines"
	};

	for ( int i = 0; i < ARRAYSIZE(sampleText); ++i)
	{
		// put it in a label
		vgui::Label *label = vgui::SETUP_PANEL(new vgui::Label(this, NULL, sampleText[i]));

		label->SetFont( m_hSmallFont );
		label->SetPaintBackgroundEnabled( false );
		label->SetPaintBorderEnabled( false );
		label->SizeToContents();
		label->SetContentAlignment( vgui::Label::a_west );
		label->SetFgColor( GetFgColor() );
		m_Labels.AddToTail( vgui::SETUP_PANEL(label) );
	}
#endif

	// find the the widest line to set our width
	//int widest = 0;
	//for (int i = 0; i < m_Labels.Count(); i++) {
	//	vgui::Label *label = m_Labels[i];
	//	if (label->GetWide() > widest) {
	//		widest = label->GetWide();
	//	}
	//}

	// position the labels
	int col_x = m_iTextX;
	int col_y = m_iTextY;

	for (int i = 0; i < m_Labels.Count(); i++) {
		vgui::Label *label = m_Labels[i];
		int tall = label->GetTall();
		label->SetPos(col_x, col_y);

		col_y += tall + m_iTextGapY;
	}

	int ox, oy;
	GetSize(ox, oy);
 	SetSize( ox, col_y );

	m_iBaseY = col_y;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Activates the hint display
//-----------------------------------------------------------------------------
void CHudHintKeyDisplay::MsgFunc_KeyHintText( bf_read &msg )
{
	// how many strings do we receive ?
	int count = msg.ReadByte();

	// here we expect only one string
	if ( count != 1 )
	{
		DevMsg("CHudHintKeyDisplay::MsgFunc_KeyHintText: string count != 1.\n");
		return;
	}

	// read the string
	char szString[2048];
	msg.ReadString( szString, sizeof(szString) );

	// make it visible
	if ( SetHintText( szString ) )
	{
		SetVisible( true );
 		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "KeyHintMessageShow" ); 
	}
	else
	{
		// it's being cleared, hide the panel
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "KeyHintMessageHide" ); 
	}
}
