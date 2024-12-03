#pragma once

constexpr auto SURF_WORKSHOP_ADDONS_ID = "3377250885";

#include <surf/zones/surf_zones.h>

class CSurfForward : public CBaseForward<CSurfForward> {
public:
	virtual bool OnTimerStart(CSurfPlayer* player) {
		return true;
	}

	virtual bool OnTimerEnd(CSurfPlayer* player) {
		return true;
	}

	virtual bool OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* player) {
		return true;
	}

	virtual bool OnStayZone(const ZoneCache_t& zone, CSurfPlayer* player) {
		return true;
	}

	virtual bool OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* player) {
		return true;
	}
};
