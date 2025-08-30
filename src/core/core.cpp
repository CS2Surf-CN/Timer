import surf.core.interfaces;

#include <sdk/entity/cgameresourceservice.h>

CGameEntitySystem* GameEntitySystem() {
	return IFACE::pGameResourceService->GetGameEntitySystem();
}
