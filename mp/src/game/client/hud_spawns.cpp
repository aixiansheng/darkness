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

#include "ConVar.h"

#include "c_hl2mp_player.h"
#include "c_team.h"

#include "tier0/memdbgon.h"

//
// if vgui ever starts making sense, get this working again:
//
//	teleporter_tex_id = surface()->CreateNewTextureID();
//	egg_tex_id = surface()->CreateNewTextureID();
//
//	surface()->DrawSetTextureFile(teleporter_tex_id, TELEPORTER_ICON, true, true);
//	surface()->DrawSetTextureFile(egg_tex_id, EGG_ICON, true, true);
//
//	surface()->DrawSetTexture(teleporter_tex_id);
//	surface()->DrawTexturedRect(0, 0, 64, 64);
//
//

#define TELEPORTER_ICON "hud/teleporter_icon"
#define EGG_ICON		"hud/egg_icon"

class CHudSpawns : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudSpawns, CHudNumericDisplay );

public:
	CHudSpawns( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink(void);

private:
	int		m_iPoints;
};	

DECLARE_HUDELEMENT( CHudSpawns );

CHudSpawns::CHudSpawns( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudSpawns") {
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudSpawns::Init() {
	Reset();
}

void CHudSpawns::Reset() {
	SetLabelText(L"SPAWNS");
	SetDisplayValue(m_iPoints);
}

void CHudSpawns::VidInit() {
	Reset();
}

void CHudSpawns::OnThink(void) {
	C_HL2MP_Player *p;

	p = ToHL2MPPlayer(C_BasePlayer::GetLocalPlayer());
	if (!p)
		return;

	SetDisplayValue(p->GetTeam()->NumSpawnPoints());
}