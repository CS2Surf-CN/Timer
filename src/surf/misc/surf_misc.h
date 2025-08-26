#pragma once

#include <pch.h>
#include <surf/surf_player.h>

class CSurfMiscPlugin : CMovementForward, CFeatureForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnActivateServer(CNetworkGameServerBase* pGameServer) override;
	virtual void OnWeaponDropPost(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon, const int& iDropType, const Vector* targetPos) override;
	virtual void OnWeaponSwitchPost(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon) override;
	virtual void OnEntitySpawned(CEntityInstance* pEntity) override;
	virtual void OnClientDisconnect(ISource2GameClients* pClient, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) override;
	virtual bool OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) override;
	virtual bool OnPlayerMove(CCSPlayer_MovementServices* ms, CMoveData* mv) override;
	virtual bool OnTakeDamage(CCSPlayerPawnBase* pVictim, CTakeDamageInfo* info) override;
	virtual void OnResourcePrecache(IEntityResourceManifest* pResourceManifest) override;

private:
	void TweakCvars();
	void HookEvents();
	void RegisterCommands();

public:
	std::vector<CHandle<CBaseTrigger>> m_vTriggers;
	std::vector<CHandle<CBaseEntity>> m_vTeleDestination;
};

class CSurfMiscService : CSurfPlayerService {
	using CSurfPlayerService::CSurfPlayerService;

	virtual void OnReset() override;

public: // rampfix
	bool didTPM {};
	bool overrideTPM {};
	Vector tpmVelocity = vec3_invalid;
	Vector tpmOrigin = vec3_invalid;
	Vector lastValidPlane = vec3_origin;

public:
	bool m_bHide = false;
	bool m_bHideWeapons = false;
	bool m_bHideLegs = true;
	bool m_bShowTrigger = false;

public:
	void HideLegs();
	void HideWeapons();
};

namespace SURF {
	extern CSurfMiscPlugin* MiscPlugin();
} // namespace SURF
