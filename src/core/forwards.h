#pragma once

#include <pch.h>

#include <sdk/forwardbase.h>
#include <sdk/usercmd.h>
#include <sdk/entity/ccsplayerpawn.h>
#include <sdk/entity/services.h>
#include <sdk/entity/entityresource.h>

class CServerSideClient;

#define FORWARD_INIT(forwardClass) \
	template<> \
	forwardClass* CBaseForward<forwardClass>::m_pFirst = nullptr;

#define FORWARD_POST(forwardClass, forwardFn, ...) \
	for (auto p = forwardClass::m_pFirst; p; p = p->m_pNext) { \
		p->forwardFn(__VA_ARGS__); \
	}

class CCoreForward : public CBaseForward<CCoreForward> {
public:
	virtual void OnPluginStart() {}

	virtual void OnLevelInit(const char* pMapName) {}

	virtual void OnStartupServer(INetworkServerService* pServerService, const GameSessionConfiguration_t& config) {}

	virtual void OnActivateServer(CNetworkGameServerBase* pGameServer) {}

	virtual void OnApplyGameSettings(ISource2Server* pServer, KeyValues* pKV) {}

	virtual void OnGameFrame(ISource2Server* pServer, bool simulating, bool bFirstTick, bool bLastTick) {}

	virtual void OnServerGamePostSimulate(IGameSystem* pGameEvent) {}

	virtual void OnEntitySpawned(CEntityInstance* pEntity) {}

	virtual void OnEntityDeleted(CEntityInstance* pEntity) {}

	virtual void OnClientConnected(ISource2GameClients* pClient, CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress, bool bFakePlayer) {}

	virtual void OnClientFullyConnect(ISource2GameClients* pClient, CPlayerSlot slot) {}

	virtual void OnClientPutInServer(ISource2GameClients* pClient, CPlayerSlot slot, char const* pszName, int type, uint64 xuid) {}

	virtual void OnClientActive(ISource2GameClients* pClient, CPlayerSlot slot, bool bLoadGame, const char* pszName, uint64 xuid) {}

	virtual void OnClientDisconnect(ISource2GameClients* pClient, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) {}

	virtual void OnClientVoice(ISource2GameClients* pClient, CPlayerSlot slot) {}

	virtual bool OnClientCommand(ISource2GameClients* pClient, CPlayerSlot slot, const CCommand& args) {
		return true;
	}

	virtual bool OnDispatchConCommand(ICvar* pCvar, ConCommandRef cmd, const CCommandContext& ctx, const CCommand& args) {
		return true;
	}

	virtual void OnPostEventAbstract(IGameEventSystem* pEventSystem, CSplitScreenSlot nSlot, int nClientCount, const uint64* clients, INetworkMessageInternal* pEvent, const CNetMessage* pData) {}

	virtual bool OnWeaponDrop(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon, int& iDropType, Vector* targetPos) {
		return true;
	}

	virtual void OnWeaponDropPost(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon, const int& iDropType, const Vector* targetPos) {}

	virtual void OnResourcePrecache(IEntityResourceManifest* pResourceManifest) {};

	virtual bool OnTakeDamage(CCSPlayerPawn* pVictim, CTakeDamageInfo* pInfo) {
		return true;
	}

	virtual void OnTakeDamagePost(CCSPlayerPawn* pVictim, CTakeDamageInfo* pInfo) {}

	virtual void OnClientSendSnapshotBefore(CServerSideClient* pClient) {}
};
