#include "memory.h"
#include <core/gamedata.h>
#include <core/interfaces.h>
#include <core/eventmanager.h>

#include <sdk/usercmd.h>
#include <sdk/entity/services.h>
#include <sdk/entity/ccsplayerpawn.h>
#include <utils/ctimer.h>
#include <utils/utils.h>

#include <cs2surf.h>

PLUGIN_GLOBALVARS();

template<typename T>
struct ReturnType;

template<typename Ret, typename... Args>
struct ReturnType<Ret (*)(Args...)> {
	using type = Ret;
};

#define CALL_SIG(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetMemSig(sig); \
	SURF_ASSERT(fnSig); \
	using FunctionType = decltype(&fnCurrent); \
	using ReturnTypeValue = ReturnType<FunctionType>::type; \
	return MEM::SDKCall<ReturnTypeValue>(fnSig, __VA_ARGS__);

#define CALL_ADDRESS(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetAddress(sig); \
	SURF_ASSERT(fnSig); \
	using FunctionType = decltype(&fnCurrent); \
	using ReturnTypeValue = ReturnType<FunctionType>::type; \
	return MEM::SDKCall<ReturnTypeValue>(fnSig, __VA_ARGS__);

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

#pragma endregion

#pragma region hooks

// clang-format off
SH_DECL_HOOK1_void(IGameSystem, ServerGamePostSimulate, SH_NOATTRIB, false, const EventServerGamePostSimulate_t*);
static void Hook_OnServerGamePostSimulate(const EventServerGamePostSimulate_t* a2) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnServerGamePostSimulate(META_IFACEPTR(IGameSystem));
	}
}

SH_DECL_HOOK3_void(ISource2Server, GameFrame, SH_NOATTRIB, false, bool, bool, bool);
static void Hook_OnGameFrame(bool simulating, bool bFirstTick, bool bLastTick) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnGameFrame(META_IFACEPTR(ISource2Server), simulating, bFirstTick, bLastTick);
	}
}

SH_DECL_HOOK6(ISource2GameClients, ClientConnect, SH_NOATTRIB, false, bool, CPlayerSlot, const char*, uint64, const char*, bool, CBufferString*);
static bool Hook_ClientConnect(CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, bool unk1,
							   CBufferString* pRejectReason) {
	RETURN_META_VALUE(MRES_IGNORED, true);
}

SH_DECL_HOOK6_void(ISource2GameClients, OnClientConnected, SH_NOATTRIB, false, CPlayerSlot, const char *, uint64, const char *, const char *, bool);
static void Hook_OnClientConnected(CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress,
								   bool bFakePlayer) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientConnected(META_IFACEPTR(ISource2GameClients), slot, pszName, xuid, pszNetworkID, pszAddress, bFakePlayer);
	}

	RETURN_META(MRES_IGNORED);
}

SH_DECL_HOOK1_void(ISource2GameClients, ClientFullyConnect, SH_NOATTRIB, false, CPlayerSlot);
static void Hook_ClientFullyConnect(CPlayerSlot slot) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientFullyConnect(META_IFACEPTR(ISource2GameClients), slot);
	}

	RETURN_META(MRES_IGNORED);
}

SH_DECL_HOOK4_void(ISource2GameClients, ClientPutInServer, SH_NOATTRIB, false, CPlayerSlot, char const *, int, uint64);
static void Hook_ClientPutInServer(CPlayerSlot slot, char const* pszName, int type, uint64 xuid) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientPutInServer(META_IFACEPTR(ISource2GameClients), slot, pszName, type, xuid);
	}

	RETURN_META(MRES_IGNORED);
}

SH_DECL_HOOK4_void(ISource2GameClients, ClientActive, SH_NOATTRIB, false, CPlayerSlot, bool, const char *, uint64);
static void Hook_ClientActive(CPlayerSlot slot, bool bLoadGame, const char* pszName, uint64 xuid) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientActive(META_IFACEPTR(ISource2GameClients), slot, bLoadGame, pszName, xuid);
	}

	RETURN_META(MRES_IGNORED);
}

SH_DECL_HOOK5_void(ISource2GameClients, ClientDisconnect, SH_NOATTRIB, false, CPlayerSlot, ENetworkDisconnectionReason, const char *, uint64, const char *);
static void Hook_ClientDisconnect(CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientDisconnect(META_IFACEPTR(ISource2GameClients), slot, reason, pszName, xuid, pszNetworkID);
	}

	RETURN_META(MRES_IGNORED);
}

SH_DECL_HOOK1_void(ISource2GameClients, ClientVoice, SH_NOATTRIB, false, CPlayerSlot);
static void Hook_ClientVoice(CPlayerSlot slot) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientVoice(META_IFACEPTR(ISource2GameClients), slot);
	}
}

SH_DECL_HOOK2_void(ISource2GameClients, ClientCommand, SH_NOATTRIB, false, CPlayerSlot, const CCommand &);
static void Hook_ClientCommand(CPlayerSlot slot, const CCommand& args) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnClientCommand(META_IFACEPTR(ISource2GameClients), slot, args);
	}

	RETURN_META(MRES_IGNORED);
}

SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
static void Hook_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession*, const char*) {
	SurfPlugin()->AddonInit();

	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnStartupServer(META_IFACEPTR(INetworkServerService), config);
	}

	RETURN_META(MRES_IGNORED);
}

SH_DECL_HOOK3_void(ICvar, DispatchConCommand, SH_NOATTRIB, 0, ConCommandHandle, const CCommandContext &, const CCommand &);
static void Hook_DispatchConCommand(ConCommandHandle cmd, const CCommandContext& ctx, const CCommand& args) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnDispatchConCommand(META_IFACEPTR(ICvar), cmd, ctx, args);
	}
}

SH_DECL_HOOK8_void(IGameEventSystem, PostEventAbstract, SH_NOATTRIB, 0, CSplitScreenSlot, bool, int, const uint64 *, INetworkMessageInternal *, const CNetMessage *, unsigned long, NetChannelBufType_t);
static void Hook_PostEvent(CSplitScreenSlot nSlot, bool bLocalOnly, int nClientCount, const uint64* clients, INetworkMessageInternal* pEvent,
						   const CNetMessage* pData, unsigned long nSize, NetChannelBufType_t bufType) {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnPostEventAbstract(META_IFACEPTR(IGameEventSystem), nSlot, nClientCount, clients, pEvent, pData);
	}
}

SH_DECL_HOOK0(CNetworkGameServerBase, ActivateServer, SH_NOATTRIB, false, bool);
static bool Hook_ActivateServer() {
	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
		p->OnActivateServer(META_IFACEPTR(CNetworkGameServerBase));
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}

// clang-format on

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

#pragma endregion

#pragma region setup

static bool SetupDetours() {
	// clang-format off
	HOOK_SIG("CCSPlayer_WeaponServices::Weapon_Drop", Hook_OnWeaponDrop, MEM::TRAMPOLINE::g_fnWeaponDrop);
	HOOK_SIG("CCSPlayerPawn::OnTakeDamage", Hook_OnTakeDamage, MEM::TRAMPOLINE::g_fnTakeDamage)
	// clang-format on

	return true;
}

static bool SetupVMTHooks() {
	// clang-format off
	HOOK_VMT("CGameEventManager::CreateEvent", MEM::MODULE::server, Hook_OnCreateEvent, MEM::TRAMPOLINE::g_fnCreateGameEvent);
	HOOK_VMT("CGameEventManager::FireEvent", MEM::MODULE::server, Hook_OnFireEvent, MEM::TRAMPOLINE::g_fnFireGameEvent);

	// clang-format on
	return true;
}

static bool SetupSourceHooks() {
	SH_ADD_HOOK(ISource2Server, GameFrame, IFACE::pServer, SH_STATIC(Hook_OnGameFrame), false);

	SH_ADD_HOOK(ISource2GameClients, ClientConnect, g_pSource2GameClients, SH_STATIC(Hook_ClientConnect), false);
	SH_ADD_HOOK(ISource2GameClients, OnClientConnected, g_pSource2GameClients, SH_STATIC(Hook_OnClientConnected), false);
	SH_ADD_HOOK(ISource2GameClients, ClientFullyConnect, g_pSource2GameClients, SH_STATIC(Hook_ClientFullyConnect), false);
	SH_ADD_HOOK(ISource2GameClients, ClientPutInServer, g_pSource2GameClients, SH_STATIC(Hook_ClientPutInServer), false);
	SH_ADD_HOOK(ISource2GameClients, ClientActive, g_pSource2GameClients, SH_STATIC(Hook_ClientActive), false);
	SH_ADD_HOOK(ISource2GameClients, ClientDisconnect, g_pSource2GameClients, SH_STATIC(Hook_ClientDisconnect), true);
	SH_ADD_HOOK(ISource2GameClients, ClientVoice, g_pSource2GameClients, SH_STATIC(Hook_ClientVoice), false);
	SH_ADD_HOOK(ISource2GameClients, ClientCommand, g_pSource2GameClients, SH_STATIC(Hook_ClientCommand), false);

	SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_STATIC(Hook_StartupServer), true);

	SH_ADD_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_STATIC(Hook_DispatchConCommand), false);

	SH_ADD_HOOK(IGameEventSystem, PostEventAbstract, IFACE::pGameEventSystem, SH_STATIC(Hook_PostEvent), false);

	// clang-format off
	SH_ADD_DVPHOOK(IGameSystem, 
		ServerGamePostSimulate, 
		MEM::MODULE::server->GetVirtualTableByName("CEntityDebugGameSystem").RCast<IGameSystem*>(), 
		SH_STATIC(Hook_OnServerGamePostSimulate), 
		true
	);

	SH_ADD_DVPHOOK(
		CNetworkGameServerBase, 
		ActivateServer,
		MEM::MODULE::engine->GetVirtualTableByName("CNetworkGameServer").RCast<CNetworkGameServerBase*>(),
		SH_STATIC(Hook_ActivateServer), 
		true
	);
	// clang-format on
	return true;
}

void MEM::SetupHooks() {
	SURF_ASSERT(SetupDetours());
	SURF_ASSERT(SetupVMTHooks());
	SURF_ASSERT(SetupSourceHooks());
}

void MEM::MODULE::Setup() {
	engine = std::make_shared<libmodule::CModule>();
	engine->InitFromName(LIB::engine2, true);

	tier0 = std::make_shared<libmodule::CModule>();
	tier0->InitFromName(LIB::tier0, true);

	server = std::make_shared<libmodule::CModule>();
	server->InitFromMemory(libmem::GetModule(LIB::server).base);

	schemasystem = std::make_shared<libmodule::CModule>();
	schemasystem->InitFromName(LIB::schemasystem, true);

	steamnetworkingsockets = std::make_shared<libmodule::CModule>();
	steamnetworkingsockets->InitFromName(LIB::steamnetworkingsockets, true);
}

#pragma endregion

void MEM::SDKHOOK::StartTouchPost(CBaseEntity* pEnt, HookTouch_t pFn) {
	static int iStartTouchOffset = GAMEDATA::GetOffset("CBaseEntity::StartTouch");
	libmem::VmtHook(pEnt, iStartTouchOffset, pFn);
}

void MEM::SDKHOOK::TouchPost(CBaseEntity* pEnt, HookTouch_t pFn) {
	static int iTouchOffset = GAMEDATA::GetOffset("CBaseEntity::Touch");
	libmem::VmtHook(pEnt, iTouchOffset, pFn);
}

void MEM::SDKHOOK::EndTouchPost(CBaseEntity* pEnt, HookTouch_t pFn) {
	static int iEndTouchOffset = GAMEDATA::GetOffset("CBaseEntity::EndTouch");
	libmem::VmtHook(pEnt, iEndTouchOffset, pFn);
}
