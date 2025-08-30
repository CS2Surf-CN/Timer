import surf.core;

#include "cgameresourceservice.h"

CGameEntitySystem* CGameResourceService::GetGameEntitySystem() {
	return *reinterpret_cast<CGameEntitySystem**>((char*)(this) + GAMEDATA::GetOffset("GameEntitySystem"));
}
