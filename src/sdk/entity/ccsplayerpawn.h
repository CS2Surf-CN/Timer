#pragma once

#include "cbaseplayerpawn.h"

class CCSPlayer_ViewModelServices;
class CCSBot;
class CCSPlayerController;

enum class CSPlayerState : uint32_t {
	STATE_ACTIVE = 0x0,
	STATE_WELCOME = 0x1,
	STATE_PICKINGTEAM = 0x2,
	STATE_PICKINGCLASS = 0x3,
	STATE_DEATH_ANIM = 0x4,
	STATE_DEATH_WAIT_FOR_KEY = 0x5,
	STATE_OBSERVER_MODE = 0x6,
	STATE_GUNGAME_RESPAWN = 0x7,
	STATE_DORMANT = 0x8,
	NUM_PLAYER_STATES = 0x9,
};

class CCSPlayerPawnBase : public CBasePlayerPawn {
public:
	DECLARE_SCHEMA_CLASS(CCSPlayerPawnBase);

	SCHEMA_FIELD(CHandle<CCSPlayerController>, m_hOriginalController);
	SCHEMA_FIELD(CSPlayerState, m_iPlayerState);

	Vector GetEyePosition();
	QAngle GetEyeAngle();

	bool IsObserverActive() {
		return m_iPlayerState() == CSPlayerState::STATE_OBSERVER_MODE;
	}
};

class CCSPlayerPawn : public CCSPlayerPawnBase {
public:
	DECLARE_SCHEMA_CLASS(CCSPlayerPawn);
	SCHEMA_FIELD(float, m_ignoreLadderJumpTime);
	SCHEMA_FIELD(float, m_flSlopeDropOffset);
	SCHEMA_FIELD(float, m_flSlopeDropHeight);
	SCHEMA_FIELD(float, m_flVelocityModifier);
	SCHEMA_FIELD(CCSBot*, m_pBot);
	SCHEMA_FIELD(QAngle, m_aimPunchAngle);
	SCHEMA_FIELD(QAngle, m_angEyeAngles);

public:
	template<typename T = CBasePlayerController>
	T* GetController() {
		return (T*)(m_hController()->Get());
	}

	void Respawn() {
		static auto iOffset = GAMEDATA::GetOffset("Respawn");
		CALL_VIRTUAL(void, iOffset, this);
	}
};

class CBot {
public:
	DECLARE_SCHEMA_CLASS(CBot);

	SCHEMA_FIELD(uint64_t, m_buttonFlags);
	SCHEMA_FIELD(CCSPlayerPawn*, m_pPlayer);
	SCHEMA_FIELD(CCSPlayerController*, m_pController);
};

class CCSBot : public CBot {
public:
	DECLARE_SCHEMA_CLASS(CCSBot);
};
