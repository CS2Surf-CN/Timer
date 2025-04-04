#pragma once

#include <movement/movement.h>
#include <list>

class CSurfPlayer;
class CSurfTimerService;
class CSurfZoneService;
class CSurfHudService;
class CSurfReplayService;
class CSurfMiscService;
class CSurfGlobalAPIService;
class CSurfCheckpointService;

class CSurfBaseService {
public:
	CSurfBaseService(CSurfPlayer* player) : m_pPlayer(player) {}

	CSurfPlayer* GetPlayer() const {
		return m_pPlayer;
	}

public:
	virtual void OnInit() {}

	virtual void OnReset() {}

	virtual void PlayErrorSound() const;

private:
	CSurfPlayer* m_pPlayer;
};

class CSurfPlayer : public CMovementPlayer {
public:
	using CMovementPlayer::CMovementPlayer;

	virtual void Init(int iSlot) override;
	virtual void Reset() override;

private:
	struct ServiceDeleter {
		void operator()(void* ptr) const {
			delete reinterpret_cast<CSurfBaseService*>(ptr);
		}
	};

	template<typename T>
	void InitService(std::unique_ptr<T, ServiceDeleter>& service) {
		static_assert(std::is_base_of<CSurfBaseService, T>::value, "T must be derived from CSurfBaseService");

		if (!service) {
			service.reset(new T(this));
			reinterpret_cast<CSurfBaseService*>(service.get())->OnInit();
			reinterpret_cast<CSurfBaseService*>(service.get())->OnReset();
		}
	}

	template<typename T>
	void ResetService(std::unique_ptr<T, ServiceDeleter>& service) {
		static_assert(std::is_base_of<CSurfBaseService, T>::value, "T must be derived from CSurfBaseService");

		if (service) {
			reinterpret_cast<CSurfBaseService*>(service.get())->OnReset();
		}
	}

public:
	std::unique_ptr<CSurfTimerService, ServiceDeleter> m_pTimerService;
	std::unique_ptr<CSurfZoneService, ServiceDeleter> m_pZoneService;
	std::unique_ptr<CSurfHudService, ServiceDeleter> m_pHudService;
	std::unique_ptr<CSurfReplayService, ServiceDeleter> m_pReplayService;
	std::unique_ptr<CSurfMiscService, ServiceDeleter> m_pMiscService;
	std::unique_ptr<CSurfGlobalAPIService, ServiceDeleter> m_pGlobalAPIService;
	std::unique_ptr<CSurfCheckpointService, ServiceDeleter> m_pCheckpointService;

public:
	bool m_bJustTeleported;
};

class CSurfPlayerManager : public CMovementPlayerManager {
public:
	CSurfPlayerManager() {
		for (int i = 0; i < MAXPLAYERS; i++) {
			m_pPlayers[i] = std::make_unique<CSurfPlayer>(i);
		}
	}

	virtual CSurfPlayer* ToPlayer(CServerSideClientBase* pClient) const override;
	virtual CSurfPlayer* ToPlayer(CPlayerPawnComponent* component) const override;
	virtual CSurfPlayer* ToPlayer(CBasePlayerController* controller) const override;
	virtual CSurfPlayer* ToPlayer(CBasePlayerPawn* pawn) const override;
	virtual CSurfPlayer* ToPlayer(CPlayerSlot slot) const override;
	virtual CSurfPlayer* ToPlayer(CEntityIndex entIndex) const override;
	virtual CSurfPlayer* ToPlayer(CPlayerUserId userID) const override;
	virtual CSurfPlayer* ToPlayer(CSteamID steamid, bool validate = false) const override;

	// fake players are excluded
	virtual std::vector<CPlayer*> GetOnlinePlayers() const override;

	// Safe
	CSurfPlayer* ToSurfPlayer(CMovementPlayer* player) {
		return static_cast<CSurfPlayer*>(player);
	}

	// Dont pass by global playermanager
	CSurfPlayer* ToSurfPlayer(CPlayer* player) {
		return static_cast<CSurfPlayer*>(player);
	}

private:
	virtual void OnClientConnected(ISource2GameClients* pClient, CPlayerSlot slot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress, bool bFakePlayer) override;
	virtual void OnEntitySpawned(CEntityInstance* pEntity) override;
};

namespace SURF {
	extern CSurfPlayerManager* GetPlayerManager();
} // namespace SURF
