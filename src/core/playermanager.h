#pragma once

#include <pch.h>
#include <sdk/serversideclient.h>
#include <sdk/entity/observer.h>
#include <sdk/entity/ccsplayercontroller.h>
#include <core/forwards.h>

class CPlayer;

class CPlayerService {
public:
	CPlayerService() = delete;

	CPlayerService(CPlayer* player)
		: m_pPlayer(player) {}

public:
	virtual ~CPlayerService() {}

	virtual CPlayer* GetPlayer() const {
		return m_pPlayer;
	}

	virtual void OnInit() {}

	virtual void OnReset() {}

protected:
	CPlayer* m_pPlayer;
};

class CPlayerServiceFactory {
protected:
	virtual ~CPlayerServiceFactory() {}

	struct PlayerServiceDeleter {
		void operator()(void* ptr) const {
			delete reinterpret_cast<CPlayerService*>(ptr);
		}
	};

public:
	template<typename T>
	using PlayerServicePtr = std::unique_ptr<T, PlayerServiceDeleter>;

protected:
	template<typename T>
	void InitService(PlayerServicePtr<T>& service) {
		static_assert(std::is_base_of<CPlayerService, T>::value, "T must be derived from CPlayerService");

		if (!service) {
			service.reset(new T(reinterpret_cast<CPlayer*>(this)));
			reinterpret_cast<CPlayerService*>(service.get())->OnInit();
		}
	}

	template<typename T>
	void ResetService(PlayerServicePtr<T>& service) {
		static_assert(std::is_base_of<CPlayerService, T>::value, "T must be derived from CPlayerService");

		if (service) {
			reinterpret_cast<CPlayerService*>(service.get())->OnReset();
		}
	}
};

class CPlayer : public CPlayerServiceFactory {
public:
	CPlayer(int slot)
		: m_iSlot(slot) {}

	virtual ~CPlayer() {}

	virtual void Init(int slot) {
		m_iSlot = slot;
	}

	virtual void Reset() {
		m_iSlot = -1;
		unauthenticatedSteamID = k_steamIDNil;
		m_bFakeClient = false;
	}

	virtual CCSPlayerController* GetController() const;
	virtual CBasePlayerPawn* GetCurrentPawn() const;
	virtual CCSPlayerPawn* GetPlayerPawn() const;
	virtual CCSObserverPawn* GetObserverPawn() const;
	virtual CServerSideClient* GetClient() const;

	// Returns true if this client has been fully authenticated by Steam
	virtual bool IsAuthenticated() const;
	virtual bool IsConnected() const;
	virtual bool IsInGame() const;
	virtual bool IsFakeClient() const;
	virtual bool IsSourceTV() const;
	virtual bool IsAlive() const;
	virtual bool IsValidClient() const;
	virtual const char* GetName() const;
	virtual const char* GetIpAddress() const;
	virtual CSteamID GetSteamId(bool validated = false) const;

	virtual void Kick(const char* internalReason, ENetworkDisconnectionReason reason);
	virtual void SetName(const char* name);

	uint64 GetSteamId64(bool validated = false) const {
		return GetSteamId(validated).ConvertToUint64();
	}

	void SetUnauthenticatedSteamID(uint64 xuid) {
		unauthenticatedSteamID = CSteamID(xuid);
	}

	CPlayerSlot GetPlayerSlot() const {
		return m_iSlot;
	}

	CEntityIndex GetEntityIndex() const {
		return m_iSlot + 1;
	}

private:
	int m_iSlot = -1;
	CSteamID unauthenticatedSteamID = k_steamIDNil;

public:
	bool m_bFakeClient = false;
};

class CPlayerManager : CCoreForward {
public:
	CPlayerManager() {
		for (int i = 0; i < MAXPLAYERS; i++) {
			m_pPlayers[i] = std::make_unique<CPlayer>(i);
		}
	}

	virtual CPlayer* ToPlayer(CServerSideClientBase* pClient) const;
	virtual CPlayer* ToPlayer(CPlayerPawnComponent* component) const;
	virtual CPlayer* ToPlayer(CBasePlayerController* controller) const;
	virtual CPlayer* ToPlayer(CBasePlayerPawn* pawn) const;
	virtual CPlayer* ToPlayer(CPlayerSlot slot) const;
	virtual CPlayer* ToPlayer(CEntityIndex entIndex) const;
	virtual CPlayer* ToPlayer(CPlayerUserId userID) const;
	virtual CPlayer* ToPlayer(CSteamID steamid, bool validate = false) const;

	// included fake players
	virtual std::vector<CPlayer*> GetOnlinePlayers() const;

protected:
	virtual void OnClientDisconnect(ISource2GameClients* pClient, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) override;
	virtual void OnClientPutInServer(ISource2GameClients* pClient, CPlayerSlot slot, char const* pszName, int type, uint64 xuid) override;

protected:
	std::array<std::unique_ptr<CPlayer>, MAXPLAYERS> m_pPlayers;
};

extern CPlayerManager* GetPlayerManager();
