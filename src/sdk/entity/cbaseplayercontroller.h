#pragma once

#include <ehandle.h>

#include "cbaseentity.h"
#include "ccsplayerpawn.h"

class CCSPlayerPawn;

enum class PlayerConnectedState : uint32_t {
	PlayerNeverConnected = 0xFFFFFFFF,
	PlayerConnected = 0x0,
	PlayerConnecting = 0x1,
	PlayerReconnecting = 0x2,
	PlayerDisconnecting = 0x3,
	PlayerDisconnected = 0x4,
	PlayerReserved = 0x5,
};

class CBasePlayerController : public CBaseEntity {
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerController);

	SCHEMA_FIELD(uint64, m_steamID);
	SCHEMA_FIELD(bool, m_bIsHLTV);
	SCHEMA_FIELD(CHandle<CCSPlayerPawn>, m_hPawn);
	SCHEMA_FIELD(PlayerConnectedState, m_iConnected);
	SCHEMA_FIELD_STRING(m_iszPlayerName, 128);

	CBasePlayerPawn* GetCurrentPawn() {
		return m_hPawn().Get();
	}

	const char* GetPlayerName() {
		return m_iszPlayerName();
	}

	void SetPlayerName(const char* pszName);

	int GetPlayerSlot() {
		return entindex() - 1;
	}

	PlayerConnectedState GetConnectedState() {
		return m_iConnected();
	}

	bool InGame() {
		return GetConnectedState() == PlayerConnectedState::PlayerConnected;
	}

	bool IsBot() {
		return m_steamID() == 0;
	}

	bool IsObserver();

	void SetPawn(CCSPlayerPawn* pawn);
	void SendFullUpdate();
};
