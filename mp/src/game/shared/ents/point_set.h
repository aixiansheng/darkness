#ifndef __POINT_SET_H__
#define __POINT_SET_H__

#include "cbase.h"

class CPointSet : public CLogicalEntity {
public:
	DECLARE_CLASS(CPointSet, CLogicalEntity);
	DECLARE_DATADESC();

	CPointSet(void);
	~CPointSet(void);

	int SpiderPoints(void);
	int HumanPoints(void);

private:
	int spiderPoints;
	int humanPoints;
	int foo;
};

#endif