#pragma once

#include <surf/surf_player.h>

class CSurfZoneService : CSurfBaseService {
public:
	using CSurfBaseService::CSurfBaseService;

private:
	virtual void OnServiceSetup() override;
};