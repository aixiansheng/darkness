#include "cbase.h"
#include "hud.h"
#include "hud_pack_item.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include <vgui_controls/controls.h>
#include <vgui/ISurface.h>

#include "c_hl2mp_player.h"
#include "ents/pack_item.h"

#include "tier0/memdbgon.h"


using namespace vgui;

DECLARE_HUDELEMENT(CHudPackItem);

CHudPackItem::CHudPackItem(const char *name) : CHudElement(name), BaseClass(NULL, "HudPackItem") {
	Panel *par = g_pClientMode->GetViewport();
	SetParent(par);

	SetVisible(false);
	SetAlpha(255);

	icon_idxs[PACK_ITEM_TYPE_HEALTH] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(icon_idxs[PACK_ITEM_TYPE_HEALTH], pack_item_info[PACK_ITEM_TYPE_HEALTH].icon, true, true);

	idx = -1;

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudPackItem::PaintBackground() {
	return;
}

void CHudPackItem::Paint() {
	if (idx > 0) {
		SetPaintBorderEnabled(false);
		surface()->DrawSetTexture(icon_idxs[idx]);
		surface()->DrawTexturedRect(2, 2, 66, 66);
	}
}

void CHudPackItem::OnThink() {
	C_HL2MP_Player *p;

	BaseClass::OnThink();

	p = ToHL2MPPlayer(C_BasePlayer::GetLocalPlayer());
	if (!p)
		return;

	switch (p->pack_item_idx) {
	case 0:
		idx = p->pack_item_0;
		break;
	case 1:
		idx = p->pack_item_1;
		break;
	case 2:
		idx = p->pack_item_2;
		break;
	default:
		idx = -1;
	}

}

