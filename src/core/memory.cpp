module;

//#include <core/gamedata.h>
//#include <core/interfaces.h>
//#include <core/eventmanager.h>

//#include <sdk/usercmd.h>
//#include <sdk/entity/services.h>
//#include <sdk/entity/ccsplayerpawn.h>
#include <utils/utils.h>
#include <utils/typehelper.h>

//#include <cs2surf.h>

module surf.core.memory;

import surf.core;

//class CEntListener : public IEntityListener {
//	virtual void OnEntitySpawned(CEntityInstance* pEntity) override;
//	virtual void OnEntityDeleted(CEntityInstance* pEntity) override;
//} g_EntityListener;

MEM::CHookManager g_HookManager;

bool g_bMapStarted = false;

#define CALL_SIG(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetMemSig(sig); \
	SDK_ASSERT(fnSig); \
	return MEM::SDKCall<FunctionTraits<decltype(&fnCurrent)>::ReturnType>(fnSig, __VA_ARGS__);

#define CALL_ADDRESS(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetAddress(sig); \
	SDK_ASSERT(fnSig); \
	return MEM::SDKCall<FunctionTraits<decltype(&fnCurrent)>::ReturnType>(fnSig, __VA_ARGS__);

//#pragma region calls
//
//namespace MEM::CALL {
//	void SwitchTeam(CCSPlayerController* controller, int team) {
//		CALL_SIG("CCSPlayerController_SwitchTeam", SwitchTeam, controller, team);
//	}
//
//	void SetPawn(CBasePlayerController* controller, CCSPlayerPawn* pawn, bool a3, bool a4, bool a5) {
//		CALL_SIG("CBasePlayerController_SetPawn", SetPawn, controller, pawn, a3, a4, a5);
//	}
//
//	IGameEventListener2* GetLegacyGameEventListener(CPlayerSlot slot) {
//		CALL_SIG("GetLegacyGameEventListener", GetLegacyGameEventListener, slot);
//	}
//
//	bool TraceShape(const Ray_t& ray, const Vector& vecStart, const Vector& vecEnd, const CTraceFilter& filter, CGameTrace* tr) {
//		CALL_SIG("TraceShape", TraceShape, IFACE::pEngineTrace, &ray, &vecStart, &vecEnd, &filter, tr);
//	}
//
//	void TracePlayerBBox(const Vector& start, const Vector& end, const bbox_t& bounds, CTraceFilter* filter, trace_t& pm) {
//		CALL_SIG("TracePlayerBBox", TracePlayerBBox, &start, &end, &bounds, filter, &pm);
//	}
//
//	void SnapViewAngles(CBasePlayerPawn* pawn, const QAngle& angle) {
//		CALL_SIG("SnapViewAngles", SnapViewAngles, pawn, &angle);
//	}
//
//	void CEntityInstance_AcceptInput(CEntityInstance* pEnt, const char* pInputName, CEntityInstance* pActivator, CEntityInstance* pCaller, variant_t* value, int nOutputID) {
//		CALL_SIG("CEntityInstance_AcceptInput", CEntityInstance_AcceptInput, pEnt, pInputName, pActivator, pCaller, value, nOutputID);
//	}
//
//	CBaseEntity* CreateEntityByName(const char* pszName) {
//		CALL_SIG("CBaseEntity::CreateEntityByName", CreateEntityByName, pszName, -1);
//	}
//
//	void DispatchSpawn(CBaseEntity* pEnt, CEntityKeyValues* pInitKeyValue) {
//		CALL_SIG("CBaseEntity::DispatchSpawn", DispatchSpawn, pEnt, pInitKeyValue);
//	}
//
//	CBaseTrigger* CreateAABBTrigger(const Vector& center, const Vector& mins, const Vector& maxs) {
//		CALL_SIG("CBaseTrigger::CreateAABBTrigger", CreateAABBTrigger, &center, &mins, &maxs);
//	}
//
//	void SetParent(CBaseEntity* pEnt, CBaseEntity* pParent) {
//		CALL_SIG("CBaseEntity::SetParent", SetParent, pEnt, pParent, 0, 0);
//	}
//
//	void SetEntityName(CEntityIdentity* pEnt, const char* pszName) {
//		CALL_SIG("CEntityIdentity::SetEntityName", SetEntityName, pEnt, pszName);
//	}
//
//	SndOpEventGuid_t EmitSound(IRecipientFilter& filter, CEntityIndex ent, const EmitSound_t& params) {
//		CALL_SIG("EmitSound", EmitSound, &filter, ent, &params);
//	}
//
//	bool BotAddCommand(int team, bool isFromConsole, const char* profileName, CSWeaponType weaponType, int difficulty) {
//	#ifdef _WIN32
//		CALL_SIG("CCSBotManager::BotAddCommand", BotAddCommand, nullptr, team, isFromConsole, profileName, weaponType, difficulty);
//	#else
//		CALL_SIG("CCSBotManager::BotAddCommand", BotAddCommand, team, isFromConsole, profileName, weaponType, difficulty);
//	#endif
//	}
//}
//
//#pragma endregion

//#pragma region hooks
//
//void CEntListener::OnEntitySpawned(CEntityInstance* pEntity) {
//	if (pEntity) {
//		FORWARD_POST(CCoreForward, OnEntitySpawned, pEntity, g_bMapStarted);
//	}
//}
//
//void CEntListener::OnEntityDeleted(CEntityInstance* pEntity) {
//	if (pEntity) {
//		FORWARD_POST(CCoreForward, OnEntityDeleted, pEntity);
//	}
//}
//
//static void Hook_OnServerGamePostSimulate(IGameSystem* pThis, const EventServerGamePostSimulate_t* a2) {
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnServerGamePostSimulate, pThis, a2);
//
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnServerGamePostSimulate(pThis);
//	}
//}
//
//static void Hook_OnGameFrame(ISource2Server* pThis, bool simulating, bool bFirstTick, bool bLastTick) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnGameFrame(pThis, simulating, bFirstTick, bLastTick);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnGameFrame, pThis, simulating, bFirstTick, bLastTick);
//}
//
//static bool Hook_ClientConnect(ISource2GameClients* pThis, CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, bool unk1, CBufferString* pRejectReason) {
//	return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnClientConnect, pThis, slot, pszName, xuid, pszNetworkID, unk1, pRejectReason);
//}
//
//static void Hook_OnClientConnected(ISource2GameClients* pThis, CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress, bool bFakePlayer) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnClientConnected(pThis, slot, pszName, xuid, pszNetworkID, pszAddress, bFakePlayer);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientConnected, pThis, slot, pszName, xuid, pszNetworkID, pszAddress, bFakePlayer);
//}
//
//static void Hook_ClientFullyConnect(ISource2GameClients* pThis, CPlayerSlot slot) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnClientFullyConnect(pThis, slot);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientFullyConnect, pThis, slot);
//}
//
//static void Hook_ClientPutInServer(ISource2GameClients* pThis, CPlayerSlot slot, char const* pszName, int type, uint64 xuid) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnClientPutInServer(pThis, slot, pszName, type, xuid);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientPutInServer, pThis, slot, pszName, type, xuid);
//}
//
//static void Hook_ClientActive(ISource2GameClients* pThis, CPlayerSlot slot, bool bLoadGame, const char* pszName, uint64 xuid) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnClientActive(pThis, slot, bLoadGame, pszName, xuid);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientActive, pThis, slot, bLoadGame, pszName, xuid);
//}
//
//static void Hook_ClientDisconnect(ISource2GameClients* pThis, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnClientDisconnect(pThis, slot, reason, pszName, xuid, pszNetworkID);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientDisconnect, pThis, slot, reason, pszName, xuid, pszNetworkID);
//}
//
//static void Hook_ClientVoice(ISource2GameClients* pThis, CPlayerSlot slot) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnClientVoice(pThis, slot);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientVoice, pThis, slot);
//}
//
//static void Hook_ClientCommand(ISource2GameClients* pThis, CPlayerSlot slot, const CCommand& args) {
//	bool bBlock = false;
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		if (!p->OnClientCommand(pThis, slot, args)) {
//			bBlock = true;
//		}
//	}
//	if (bBlock) {
//		return;
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientCommand, pThis, slot, &args);
//}
//
//static void Hook_StartupServer(INetworkServerService* pThis, const GameSessionConfiguration_t& config, ISource2WorldSession* a3, const char* a4) {
//	SurfPlugin()->AddonInit();
//	auto entitySystem = GameEntitySystem();
//	entitySystem->RemoveListenerEntity(&g_EntityListener);
//	entitySystem->AddListenerEntity(&g_EntityListener);
//
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnStartupServer(pThis);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnStartupServer, pThis, &config, a3, a4);
//}
//
//static void Hook_OnLoopDeactivate(ILoopMode* pThis, const EngineLoopState_t& state, CEventDispatcher<CEventIDManager_Default>* pEventDispatcher) {
//	FORWARD_POST(CCoreForward, OnMapEnd);
//
//	g_bMapStarted = false;
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnLoopDeactivate, pThis, &state, pEventDispatcher);
//}
//
//static void Hook_DispatchConCommand(ICvar* pThis, ConCommandRef cmd, const CCommandContext& ctx, const CCommand& args) {
//	bool block = false;
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		if (!p->OnDispatchConCommand(pThis, cmd, ctx, args)) {
//			block = true;
//		}
//	}
//	if (block) {
//		return;
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnDispatchConCommand, pThis, cmd, &ctx, &args);
//}
//
//static void Hook_PostEvent(IGameEventSystem* pThis, CSplitScreenSlot nSlot, bool bLocalOnly, int nClientCount, const uint64* clients, INetworkMessageInternal* pEvent, const CNetMessage* pData, unsigned long nSize, NetChannelBufType_t bufType) {
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnPostEventAbstract(pThis, nSlot, nClientCount, clients, pEvent, pData);
//	}
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnPostEventAbstract, pThis, nSlot, bLocalOnly, nClientCount, clients, pEvent, pData, nSize, bufType);
//}
//
//static bool Hook_ActivateServer(CNetworkGameServerBase* pThis) {
//	auto ret = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnActivateServer, pThis);
//
//	for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
//		p->OnActivateServer(pThis);
//	}
//
//	g_bMapStarted = true;
//
//	return ret;
//}
//
//static IGameEvent* Hook_OnCreateEvent(IGameEventManager2* pEventManager, const char* szName, bool bForce, int* pCookie) {
//	return MEM::SDKCall<IGameEvent*>(MEM::TRAMPOLINE::g_fnCreateGameEvent, pEventManager, szName, true, pCookie);
//}
//
//static bool Hook_OnFireEvent(IGameEventManager2* pEventManager, IGameEvent* pEvent, bool bDontBroadcast) {
//	if (!pEvent) {
//		return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
//	}
//
//	auto& pList = EVENT::m_plist[std::string(pEvent->GetName())];
//	if (pList.empty()) {
//		return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
//	}
//
//	auto pClone = IFACE::pGameEventManager->DuplicateEvent(pEvent);
//	if (!pClone) {
//		return MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
//	}
//
//	auto block = false;
//	for (auto& event : pList) {
//		if (!event.m_pCallbackPre) {
//			continue;
//		}
//
//		if (!(event.m_pCallbackPre)(pEvent, pEvent->GetName(), bDontBroadcast)) {
//			block = true;
//		}
//	}
//
//	if (block) {
//		IFACE::pGameEventManager->FireEvent(pClone);
//		return false;
//	}
//
//	auto result = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnFireGameEvent, pEventManager, pEvent, bDontBroadcast);
//	for (auto& event : pList) {
//		if (event.m_pCallbackPost) {
//			(event.m_pCallbackPost)(pClone, pClone->GetName(), bDontBroadcast);
//		} else if (event.m_pCallbackPostNoCopy) {
//			(event.m_pCallbackPostNoCopy)(pClone->GetName(), bDontBroadcast);
//		}
//	}
//
//	pEventManager->FreeEvent(pClone);
//
//	return result;
//}
//
//static void Hook_OnApplyGameSettings(ISource2Server* pThis, KeyValues* pKV) {
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnApplyGameSettings, pThis, pKV);
//
//	FORWARD_POST(CCoreForward, OnApplyGameSettings, pThis, pKV);
//}
//
//static bool Hook_OnWeaponDrop(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon, int iDropType, Vector* targetPos) {
//	bool block = false;
//	for (auto p = CFeatureForward::m_pFirst; p; p = p->m_pNext) {
//		if (!p->OnWeaponDrop(pService, pWeapon, iDropType, targetPos)) {
//			block = true;
//		}
//	}
//	if (block) {
//		return false;
//	}
//
//	auto ret = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnWeaponDrop, pService, pWeapon, iDropType, targetPos);
//
//	FORWARD_POST(CFeatureForward, OnWeaponDropPost, pService, pWeapon, iDropType, targetPos);
//
//	return ret;
//}
//
//static bool Hook_OnWeaponSwitch(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon, int unk) {
//	bool block = false;
//	for (auto p = CFeatureForward::m_pFirst; p; p = p->m_pNext) {
//		if (!p->OnWeaponSwitch(pService, pWeapon)) {
//			block = true;
//		}
//	}
//	if (block) {
//		return false;
//	}
//
//	auto ret = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnWeaponSwitch, pService, pWeapon, unk);
//
//	FORWARD_POST(CFeatureForward, OnWeaponSwitchPost, pService, pWeapon);
//
//	return ret;
//}
//
//static int Hook_OnTakeDamage(CCSPlayer_DamageReactServices* pService, CTakeDamageInfo* info) {
//	auto pVictim = pService->GetPawn();
//	bool block = false;
//	for (auto p = CFeatureForward::m_pFirst; p; p = p->m_pNext) {
//		if (!p->OnTakeDamage(pVictim, info)) {
//			block = true;
//		}
//	}
//	if (block) {
//		return 1;
//	}
//
//	auto ret = MEM::SDKCall<int>(MEM::TRAMPOLINE::g_fnTakeDamage, pService, info);
//
//	FORWARD_POST(CFeatureForward, OnTakeDamagePost, pVictim, info);
//
//	return ret;
//}
//
//static void Hook_OnClientSendSnapshotBefore(CServerSideClient* pClient, void* pFrameSnapshot) {
//	FORWARD_POST(CFeatureForward, OnClientSendSnapshotBefore, pClient);
//
//	MEM::SDKCall(MEM::TRAMPOLINE::g_fnClientSendSnapshotBefore, pClient, pFrameSnapshot);
//}
//
//static bool Hook_OnSetObserverTarget(CPlayer_ObserverServices* pService, CBaseEntity* pEnt) {
//	auto ret = MEM::SDKCall<bool>(MEM::TRAMPOLINE::g_fnSetObserverTarget, pService, pEnt);
//
//	const auto iObsMode = pService->m_iObserverMode();
//	if (iObsMode == OBS_MODE_NONE) {
//		return ret;
//	}
//
//	if (!pEnt && iObsMode == OBS_MODE_IN_EYE) {
//		return ret;
//	}
//
//	FORWARD_POST(CFeatureForward, OnSetObserverTargetPost, pService, pEnt, iObsMode);
//
//	return ret;
//}
//
//#pragma endregion

#pragma region setup

static bool SetupDetours() {
	// clang-format off
	/*HOOK_SIG("CCSPlayer_WeaponServices::Weapon_Drop", Hook_OnWeaponDrop, MEM::TRAMPOLINE::g_fnWeaponDrop);
	HOOK_SIG("CCSPlayer_WeaponServices::Weapon_Switch", Hook_OnWeaponSwitch, MEM::TRAMPOLINE::g_fnWeaponSwitch);
	HOOK_SIG("CCSPlayerPawn::OnTakeDamage", Hook_OnTakeDamage, MEM::TRAMPOLINE::g_fnTakeDamage);
	HOOK_SIG("CServerSideClient::SendSnapshotBefore", Hook_OnClientSendSnapshotBefore, MEM::TRAMPOLINE::g_fnClientSendSnapshotBefore);
	HOOK_SIG("CPlayer_ObserverServices::SetObserverTarget", Hook_OnSetObserverTarget, MEM::TRAMPOLINE::g_fnSetObserverTarget);*/
	// clang-format on

	return true;
}

static bool SetupVMTHooks() {
	// clang-format off
	/*HOOK_VMT(IFACE::pServer, ISource2Server::ApplyGameSettings, Hook_OnApplyGameSettings, MEM::TRAMPOLINE::g_fnApplyGameSettings);
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
		MEM::MODULE::engine2,
		Hook_ActivateServer,
		MEM::TRAMPOLINE::g_fnActivateServer
	);

	HOOK_VMTEX(
		"CLoopModeGame",
		ILoopMode::OnLoopDeactivate,
		MEM::MODULE::server,
		Hook_OnLoopDeactivate,
		MEM::TRAMPOLINE::g_fnLoopDeactivate
	);*/

	// clang-format on

	return true;
}

void MEM::SetupHooks() {
	SDK_ASSERT(SetupDetours());
	SDK_ASSERT(SetupVMTHooks());
}

void* MEM::FindPattern(const std::string_view svPattern, const std::string_view svModuleName) {
	std::string sModuleName = svModuleName.data();
	if (auto pos = sModuleName.find(MODULE_EXT); pos != std::string::npos) {
		sModuleName.resize(pos);
	}

	if (!MODULE::g_umModules.contains(sModuleName)) {
		SDK_ASSERT(false);
		return nullptr;
	}

	const auto& pModule = MODULE::g_umModules.at(sModuleName);
	return pModule->FindPattern(svPattern).RCast<void*>();
}

MEM::CHookManager* MEM::GetHookManager() {
	return &g_HookManager;
}

void MEM::MODULE::Setup() {
	engine2 = Append(LIB::engine2);
	tier0 = Append(LIB::tier0);
	server = Append(LIB::server);
	schemasystem = Append(LIB::schemasystem);
	steamnetworkingsockets = Append(LIB::steamnetworkingsockets);
}

std::shared_ptr<libmodule::CModule> MEM::MODULE::Append(const std::string_view svModuleName) {
	std::string sModuleName = svModuleName.data();
	if (auto pos = sModuleName.find(MODULE_EXT); pos != std::string::npos) {
		sModuleName.resize(pos);
	}

	if (g_umModules.contains(sModuleName)) {
		return {};
	}

	auto mod = std::make_shared<libmodule::CModule>();
	mod->InitFromName(sModuleName);
	if (!mod->IsValid()) {
		SDK_ASSERT(false);
		return {};
	}

	g_umModules[sModuleName] = mod;

	return mod;
}

#pragma endregion
