#ifndef PACK_ITEM_H__
#define PACK_ITEM_H__

#define NUM_PACK_ITEMS 2

#define PACK_ITEM_TYPE_NULL		0
#define PACK_ITEM_TYPE_HEALTH		1
//#define PACK_ITEM_TYPE_XP_SHELLS	2

struct pack_item {
	int type;
	const char *desc;
	const char *icon;
};

extern struct pack_item pack_item_info[NUM_PACK_ITEMS];

#endif
