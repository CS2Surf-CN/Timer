#pragma once

#include <movement/movement.h>
#include "datatypes.h"

class CSurfTimerService;
class CSurfZoneService;
class CSurfHudService;
class CSurfReplayService;
class CSurfMiscService;
class CSurfGlobalAPIService;
class CSurfCheckpointService;

#undef GetCurrentTime

class CSurfPlayer : public CMovementPlayer {
public:
	using CMovementPlayer::CMovementPlayer;

	virtual void Init(int iSlot) override;
	virtual void Reset() override;

public:
	void Print(const char* fmt, ...) const;
	void PrintWarning(const char* fmt, ...) const;

	TimerStyle_t GetStyle() const;
	TimerTrack_t GetCurrentTrack() const;
	TimerStage_t GetCurrentStage() const;
	TimerTime_t GetCurrentTime() const;
	bool IsPracticeMode() const;
	bool IsStageTimer() const;

public:
	PlayerServicePtr<CSurfTimerService> m_pTimerService;
	PlayerServicePtr<CSurfZoneService> m_pZoneService;
	PlayerServicePtr<CSurfHudService> m_pHudService;
	PlayerServicePtr<CSurfReplayService> m_pReplayService;
	PlayerServicePtr<CSurfMiscService> m_pMiscService;
	PlayerServicePtr<CSurfGlobalAPIService> m_pGlobalAPIService;
	PlayerServicePtr<CSurfCheckpointService> m_pCheckpointService;

public:
	bool m_bJustTeleported;
};

class CSurfPlayerService : public CPlayerService {
public:
	using CPlayerService::CPlayerService;

	virtual CSurfPlayer* GetPlayer() const override {
		return static_cast<CSurfPlayer*>(m_pPlayer);
	}

public:
	void PlayErrorSound() const;
	void Print(const char* fmt, ...) const;
	void PrintWarning(const char* fmt, ...) const;
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

private:
	virtual void OnEntitySpawned(CEntityInstance* pEntity, bool bMapStarted) override;
};

namespace SURF {
	extern CSurfPlayerManager* GetPlayerManager();
} // namespace SURF
