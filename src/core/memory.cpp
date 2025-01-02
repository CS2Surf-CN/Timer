#include "memory.h"
#include <core/gamedata.h>
#include <core/interfaces.h>
#include <core/eventmanager.h>

#include <sdk/usercmd.h>
#include <sdk/entity/services.h>
#include <sdk/entity/ccsplayerpawn.h>
#include <utils/utils.h>
#include <utils/typehelper.h>

#include <cs2surf.h>

class CEntListener : public IEntityListener {
	virtual void OnEntitySpawned(CEntityInstance* pEntity) override;
	virtual void OnEntityDeleted(CEntityInstance* pEntity) override;
} g_EntityListener;

#define CALL_SIG(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetMemSig(sig); \
	SURF_ASSERT(fnSig); \
	return MEM::SDKCall<FunctionTraits<decltype(&fnCurrent)>::ReturnType>(fnSig, __VA_ARGS__);

#define CALL_ADDRESS(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetAddress(sig); \
	SURF_ASSERT(fnSig); \
	return MEM::SDKCall<FunctionTraits<decltype(&fnCurrent)>::ReturnType>(fnSig, __VA_ARGS__);

#pragma region calls

void MEM::CALL::SwitchTeam(CCSPlayerController* controller, int team) {
	CALL_SIG("CCSPlayerController_SwitchTeam", SwitchTeam, controller, team);
}

void MEM::CALL::SetPawn(CBasePlayerController* controller, CCSPlayerPawn* pawn, bool a3, bool a4, bool a5) {
	CALL_SIG("CBasePlayerController_SetPawn", SetPawn, controller, pawn, a3, a4, a5);
}

IGameEventListener2* MEM::CALL::GetLegacyGameEventListener(CPlayerSlot slot) {
	CALL_SIG("GetLegacyGameEventListener", GetLegacyGameEventListener, slot);
}

bool MEM::CALL::TraceShape(const Ray_t& ray, const Vector& vecStart, const Vector& vecEnd, const CTraceFilter& filter, CGameTrace* tr) {
	CALL_SIG("TraceShape", TraceShape, IFACE::pGameTraceService, &ray, &vecStart, &vecEnd, &filter, tr);
}

void MEM::CALL::TracePlayerBBox(const Vector& start, const Vector& end, const bbox_t& bounds, CTraceFilter* filter, trace_t& pm) {
	CALL_SIG("TracePlayerBBox", TracePlayerBBox, &start, &end, &bounds, filter, &pm);
}

void MEM::CALL::InitPlayerMovementTraceFilter(CTraceFilterPlayerMovementCS& pFilter, CEntityInstance* pHandleEntity, uint64 interactWith,
											  int collisionGroup) {
	CALL_SIG("InitPlayerMovementTraceFilter", InitPlayerMovementTraceFilter, &pFilter, pHandleEntity, interactWith, collisionGroup);
}

void MEM::CALL::SnapViewAngles(CBasePlayerPawn* pawn, const QAngle& angle) {
	CALL_SIG("SnapViewAngles", SnapViewAngles, pawn, &angle);
}

void MEM::CALL::CEntityInstance_AcceptInput(CEntityInstance* pEnt, const char* pInputName, CEntityInstance* pActivator, CEntityInstance* pCaller,
											variant_t* value, int nOutputID) {
	CALL_SIG("CEntityInstance_AcceptInput", CEntityInstance_AcceptInput, pEnt, pInputName, pActivator, pCaller, value, nOutputID);
}

CBaseEntity* MEM::CALL::CreateEntityByName(const char* pszName) {
	CALL_SIG("CBaseEntity::CreateEntityByName", CreateEntityByName, pszName, -1);
}

void MEM::CALL::DispatchSpawn(CBaseEntity* pEnt, CEntityKeyValues* pInitKeyValue) {
	CALL_SIG("CBaseEntity::DispatchSpawn", DispatchSpawn, pEnt, pInitKeyValue);
}

CBaseTrigger* MEM::CALL::CreateAABBTrigger(const Vector& center, const Vector& mins, const Vector& maxs) {
	CALL_SIG("CBaseTrigger::CreateAABBTrigger", CreateAABBTrigger, &center, &mins, &maxs);
}

void MEM::CALL::SetParent(CBaseEntity* pEnt, CBaseEntity* pParent) {
	CALL_SIG("CBaseEntity::SetParent", SetParent, pEnt, pParent, 0, 0);
}

#pragma endregion

#pragma region hooks

static void Hook_OnServerGamePostSimulate(IGameSystem* pThis, const EventServerGamePostSimulate_t* a2) {
	MEM::SDKCall(MEM::TRAMPOLINE::g_fnServerGamePostSimulate, pThis, a2);

	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnServerGamePostSimulate(pThis);
	}
}

static void Hook_OnGameFrame(ISource2Server* pThis, bool simulating, bool bFirstTick, bool bLastTick) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnGameFrame(pThis, simulating, bFirstTick, bLastTick);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnGameFrame, pThis, simulating, bFirstTick, bLastTick);
}

static bool Hook_ClientConnect(ISource2GameClients* pThis, CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, bool unk1,
							   CBufferString* pRejectReason) {
	return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnClientConnect, pThis, slot, pszName, xuid, pszNetworkID, unk1, pRejectReason);
}

static void Hook_OnClientConnected(ISource2GameClients* pThis, CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID,
								   const char* pszAddress,
								   bool bFakePlayer) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientConnected(pThis, slot, pszName, xuid, pszNetworkID, pszAddress, bFakePlayer);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientConnected, pThis, slot, pszName, xuid, pszNetworkID, pszAddress, bFakePlayer);
}

static void Hook_ClientFullyConnect(ISource2GameClients* pThis, CPlayerSlot slot) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientFullyConnect(pThis, slot);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientFullyConnect, pThis, slot);
}

static void Hook_ClientPutInServer(ISource2GameClients* pThis, CPlayerSlot slot, char const* pszName, int type, uint64 xuid) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientPutInServer(pThis, slot, pszName, type, xuid);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientPutInServer, pThis, slot, pszName, type, xuid);
}

static void Hook_ClientActive(ISource2GameClients* pThis, CPlayerSlot slot, bool bLoadGame, const char* pszName, uint64 xuid) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientActive(pThis, slot, bLoadGame, pszName, xuid);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientActive, pThis, slot, bLoadGame, pszName, xuid);
}

static void Hook_ClientDisconnect(ISource2GameClients* pThis, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid,
								  const char* pszNetworkID) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientDisconnect(pThis, slot, reason, pszName, xuid, pszNetworkID);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientDisconnect, pThis, slot, reason, pszName, xuid, pszNetworkID);
}

static void Hook_ClientVoice(ISource2GameClients* pThis, CPlayerSlot slot) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientVoice(pThis, slot);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientVoice, pThis, slot);
}

static void Hook_ClientCommand(ISource2GameClients* pThis, CPlayerSlot slot, const CCommand& args) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientCommand(pThis, slot, args);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientCommand, pThis, slot, &args);
}

static void Hook_StartupServer(INetworkServerService* pThis, const GameSessionConfiguration_t& config, ISource2WorldSession* a3, const char* a4) {
	SurfPlugin()->AddonInit();
	auto entitySystem = GameEntitySystem();
	entitySystem->RemoveListenerEntity(&g_EntityListener);
	entitySystem->AddListenerEntity(&g_EntityListener);

	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnStartupServer(pThis, config);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnStartupServer, pThis, &config, a3, a4);
}

static void Hook_DispatchConCommand(ICvar* pThis, ConCommandHandle cmd, const CCommandContext& ctx, const CCommand& args) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnDispatchConCommand(pThis, cmd, ctx, args);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnDispatchConCommand, pThis, cmd, &ctx, &args);
}

static void Hook_PostEvent(IGameEventSystem* pThis, CSplitScreenSlot nSlot, bool bLocalOnly, int nClientCount, const uint64* clients, INetworkMessageInternal* pEvent,
						   const CNetMessage* pData, unsigned long nSize, NetChannelBufType_t bufType) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnPostEventAbstract(pThis, nSlot, nClientCount, clients, pEvent, pData);
	}

	MEM::SDKCall(MEM::TRAMPOLINE::g_fnPostEventAbstract, pThis, nSlot, bLocalOnly, nClientCount, clients, pEvent, pData, nSize, bufType);
}

static bool Hook_ActivateServer(CNetworkGameServerBase* pThis) {
	auto ret = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnActivateServer, pThis);

	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnActivateServer(pThis);
	}

	return ret;
}

static IGameEvent* Hook_OnCreateEvent(IGameEventManager2* pEventManager, const char* szName, bool bForce, int* pCookie) {
	return MEM::SDKCall<IGameEvent*>(MEM::TRAMPOLINE::g_fnCreateGameEvent, pEventManager, szName, true, pCookie);
}

static bool Hook_OnFireEvent(IGameEventManager2* pEventManager, IGameEvent* pEvent, bool bDontBroadcast) {
	if (!pEvent) {
		return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
	}

	auto& pList = EVENT::m_plist[std::string(pEvent->GetName())];
	if (pList.empty()) {
		return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
	}

	auto pClone = IFACE::pGameEventManager->DuplicateEvent(pEvent);
	if (!pClone) {
		return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
	}

	auto dontBroadcast = false;
	for (auto& event : pList) {
		if (!event.m_pCallbackPre) {
			continue;
		}

		if ((event.m_pCallbackPre)(pEvent, pEvent->GetName(), bDontBroadcast) == EventHookAction_Block) {
			dontBroadcast = true;
		}
	}

	if (dontBroadcast) {
		IFACE::pGameEventManager->FireEvent(pClone);
		return false;
	}

	auto result = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
	for (auto& event : pList) {
		if (event.m_pCallbackPost) {
			(event.m_pCallbackPost)(pClone, pClone->GetName(), bDontBroadcast);
		} else if (event.m_pCallbackPostNoCopy) {
			(event.m_pCallbackPostNoCopy)(pClone->GetName(), bDontBroadcast);
		}
	}

	pEventManager->FreeEvent(pClone);

	return result;
}

static bool Hook_OnWeaponDrop(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon, int iDropType, Vector* targetPos) {
	bool block = false;
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnWeaponDrop(pService, pWeapon, iDropType, targetPos)) {
			block = true;
		}
	}
	if (block) {
		return false;
	}

	auto ret = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnWeaponDrop, pService, pWeapon, iDropType, targetPos);

	FORWARD_POST(CCoreForward, OnWeaponDropPost, pService, pWeapon, iDropType, targetPos);

	return ret;
}

static int Hook_OnTakeDamage(CCSPlayer_DamageReactServices* pService, CTakeDamageInfo* info) {
	auto pVictim = pService->GetPawn();
	bool block = false;
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnTakeDamage(pVictim, info)) {
			block = true;
		}
	}
	if (block) {
		return 1;
	}

	auto ret = MEM::SDKCall<int>(MEM::TRAMPOLINE::g_fnTakeDamage, pService, info);

	FORWARD_POST(CCoreForward, OnTakeDamagePost, pVictim, info);

	return ret;
}

void CEntListener::OnEntitySpawned(CEntityInstance* pEntity) {
	if (pEntity) {
		FORWARD_POST(CCoreForward, OnEntitySpawned, pEntity);
	}
}

void CEntListener::OnEntityDeleted(CEntityInstance* pEntity) {
	if (pEntity) {
		FORWARD_POST(CCoreForward, OnEntityDeleted, pEntity);
	}
}

#pragma endregion

#pragma region setup

static bool SetupDetours() {
	// clang-format off
	HOOK_SIG("CCSPlayer_WeaponServices::Weapon_Drop", Hook_OnWeaponDrop, MEM::TRAMPOLINE::g_fnWeaponDrop);
	HOOK_SIG("CCSPlayerPawn::OnTakeDamage", Hook_OnTakeDamage, MEM::TRAMPOLINE::g_fnTakeDamage);
	// clang-format on

	return true;
}

static bool SetupVMTHooks() {
	// clang-format off
	HOOK_VMT(IFACE::pServer, ISource2Server::GameFrame, Hook_OnGameFrame, MEM::TRAMPOLINE::g_fnGameFrame);

	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::ClientConnect, Hook_ClientConnect, MEM::TRAMPOLINE::g_fnClientConnect);
	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::OnClientConnected, Hook_OnClientConnected, MEM::TRAMPOLINE::g_fnClientConnected);
	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::ClientFullyConnect, Hook_ClientFullyConnect, MEM::TRAMPOLINE::g_fnClientFullyConnect);
	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::ClientPutInServer, Hook_ClientPutInServer, MEM::TRAMPOLINE::g_fnClientPutInServer);
	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::ClientActive, Hook_ClientActive, MEM::TRAMPOLINE::g_fnClientActive);
	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::ClientDisconnect, Hook_ClientDisconnect, MEM::TRAMPOLINE::g_fnClientDisconnect);
	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::ClientVoice, Hook_ClientVoice, MEM::TRAMPOLINE::g_fnClientVoice);
	HOOK_VMT(g_pSource2GameClients, ISource2GameClients::ClientCommand, Hook_ClientCommand, MEM::TRAMPOLINE::g_fnClientCommand);

	HOOK_VMT(IFACE::pGameEventManager, IGameEventManager2::CreateEvent, Hook_OnCreateEvent, MEM::TRAMPOLINE::g_fnCreateGameEvent);
	HOOK_VMT(IFACE::pGameEventManager, IGameEventManager2::FireEvent, Hook_OnFireEvent, MEM::TRAMPOLINE::g_fnFireGameEvent);

	HOOK_VMT(g_pNetworkServerService, INetworkServerService::StartupServer, Hook_StartupServer, MEM::TRAMPOLINE::g_fnStartupServer);
	HOOK_VMT(g_pCVar, ICvar::DispatchConCommand, Hook_DispatchConCommand, MEM::TRAMPOLINE::g_fnDispatchConCommand);

	HOOK_VMT_OVERRIDE(IFACE::pGameEventSystem, IGameEventSystem, PostEventAbstract, Hook_PostEvent, MEM::TRAMPOLINE::g_fnPostEventAbstract, 
		CSplitScreenSlot, bool, int, const uint64*, INetworkMessageInternal*, const CNetMessage*, unsigned long, NetChannelBufType_t);
	
	HOOK_VMTEX(
		"CEntityDebugGameSystem",
		IGameSystem::ServerGamePostSimulate,
		MEM::MODULE::server,
		Hook_OnServerGamePostSimulate,
		MEM::TRAMPOLINE::g_fnServerGamePostSimulate
	);

	HOOK_VMTEX(
		"CNetworkGameServer",
		CNetworkGameServerBase::ActivateServer,
		MEM::MODULE::engine,
		Hook_ActivateServer,
		MEM::TRAMPOLINE::g_fnActivateServer
	);

	// clang-format on

	return true;
}

void MEM::SetupHooks() {
	SURF_ASSERT(SetupDetours());
	SURF_ASSERT(SetupVMTHooks());
}

void MEM::MODULE::Setup() {
	engine = std::make_shared<libmodule::CModule>();
	engine->InitFromName(LIB::engine2, true);

	tier0 = std::make_shared<libmodule::CModule>();
	tier0->InitFromName(LIB::tier0, true);

	server = std::make_shared<libmodule::CModule>();
	server->InitFromMemory(libmem::GetModule(LIB::server).base);
	SURF_ASSERT(server->GetModuleBase().GetPtr());

	schemasystem = std::make_shared<libmodule::CModule>();
	schemasystem->InitFromName(LIB::schemasystem, true);

	steamnetworkingsockets = std::make_shared<libmodule::CModule>();
	steamnetworkingsockets->InitFromName(LIB::steamnetworkingsockets, true);
}

#pragma endregion
