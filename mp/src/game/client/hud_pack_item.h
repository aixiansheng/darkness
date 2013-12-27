#ifndef HUD_PACK_ITEM_H
#define HUD_PACK_ITEM_H

#include "hudelement.h"
#include "ents/pack_item.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudPackItem : public CHudElement, public Panel {

	DECLARE_CLASS_SIMPLE(CHudPackItem, Panel);

	public:
	
	CHudPackItem(const char *name);
	
	virtual void OnThink();

	protected:
	virtual void Paint();
	virtual void PaintBackground();
	
	int icon_idxs[NUM_PACK_ITEMS];

	int idx;

};

#endif
