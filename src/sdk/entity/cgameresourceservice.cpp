#include "cgameresourceservice.h"

#include <core/gamedata.h>

CGameEntitySystem* CGameResourceService::GetGameEntitySystem() {
	return *reinterpret_cast<CGameEntitySystem**>((uintptr_t)(this) + GAMEDATA::GetOffset("GameEntitySystem"));
}
