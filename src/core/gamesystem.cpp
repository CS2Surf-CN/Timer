#include "gamesystem.h"
#include <core/gamedata.h>
#include <core/logger.h>
#include <core/forwards.h>
#include <igamesystemfactory.h>
#include <igamesystem.h>

class CGameSystem : public CBaseGameSystem {
public:
	DECLARE_GAME_SYSTEM();

	GS_EVENT(BuildGameSessionManifest);

	virtual void Shutdown() override;
	virtual void SetGameSystemGlobalPtrs(void* pValue) override;
	virtual bool DoesGameSystemReallocate() override;

	static IGameSystemFactory* sm_Factory;
};

CBaseGameSystemFactory** CBaseGameSystemFactory::sm_pFirst = nullptr;

CGameSystem g_GameSystem;
IGameSystemFactory* CGameSystem::sm_Factory = nullptr;

void GS::Setup() {
	CBaseGameSystemFactory::sm_pFirst = (CBaseGameSystemFactory**)GAMEDATA::GetAddress("GetGameSystemFactory");
	SDK_ASSERT(CBaseGameSystemFactory::sm_pFirst);

	CGameSystem::sm_Factory = new CGameSystemStaticFactory<CGameSystem>("CS2Surf_GameSystem", &g_GameSystem);
}

GS_EVENT_MEMBER(CGameSystem, BuildGameSessionManifest) {
	LOG::Print("CGameSystem::BuildGameSessionManifest\n");

	FORWARD_POST(CCoreForward, OnResourcePrecache, msg->m_pResourceManifest);
}

void CGameSystem::Shutdown() {
	LOG::Print("CGameSystem::Shutdown\n");
	g_pMemAlloc->Free(sm_Factory);
}

void CGameSystem::SetGameSystemGlobalPtrs(void* pValue) {
	if (sm_Factory) {
		sm_Factory->SetGlobalPtr(pValue);
	}
}

bool CGameSystem::DoesGameSystemReallocate() {
	return sm_Factory->ShouldAutoAdd();
}
