#include "cbase.h"
#include "ents/point_set.h"
#include "tier0/memdbgon.h"

#define DEFAULT_TEAM_POINTS 1000

LINK_ENTITY_TO_CLASS(point_set, CPointSet);

BEGIN_DATADESC(CPointSet)
	DEFINE_KEYFIELD(spiderPoints, FIELD_INTEGER, "spiderPoints" ),
	DEFINE_KEYFIELD(humanPoints, FIELD_INTEGER , "humanPoints" ),
END_DATADESC()

CPointSet::CPointSet(void) {}
CPointSet::~CPointSet(void) {}

int CPointSet::SpiderPoints(void) {
	if (spiderPoints <= 0)
		return DEFAULT_TEAM_POINTS;

	return spiderPoints;
}

int CPointSet::HumanPoints(void) {
	if (humanPoints <= 0)
		return DEFAULT_TEAM_POINTS;

	return humanPoints;
}