#pragma once

#include "cbaseplayercontroller.h"
#include "random.h"

enum OverlapState : uint8 {
	PERFECT = 0x1,
	OVERLAP = 0x2,
	UNDERLAP = 0x3,
};

struct OverlapBuffer {
	CUtlVector<OverlapState> sequences;
	uint32 current;
};

struct CStrafeStats {
	OverlapBuffer buffer;
	uint32 overlaps[16];
	uint32 underlaps[16];
};

class CCSPlayerController : public CBasePlayerController {
public:
	DECLARE_SCHEMA_CLASS(CCSPlayerController);
	SCHEMA_FIELD(CHandle<CCSPlayerPawn>, m_hPlayerPawn);
	SCHEMA_FIELD(CHandle<CCSPlayerPawnBase>, m_hObserverPawn);
	SCHEMA_FIELD_POINTER_CUSTOM(CStrafeStats, m_nNonSuspiciousHitStreak, 4);
	SCHEMA_FIELD(GameTime_t, m_LastTimePlayerWasDisconnectedForPawnsRemove);
	SCHEMA_FIELD(bool, m_bPawnIsAlive);

	CStrafeStats* GetCStrafeStats() {
		return m_nNonSuspiciousHitStreak();
	}

	CCSPlayerPawn* GetPlayerPawn() {
		return m_hPlayerPawn().Get();
	}

	CCSPlayerPawnBase* GetObserverPawn() {
		return m_hObserverPawn().Get();
	}

	void ChangeTeam(int iTeam);

	void SwitchTeam(int iTeam);

	// Respawns the player if the player is not alive, does nothing otherwise.
	void Respawn();

	// Returns the player self if the player is not an observer
	CBaseEntity* GetObserverTarget();
};
