#pragma once

#include "cbasemodelentity.h"

class CCSPlayer_WaterServices;
class CPlayer_MovementServices;
class CPlayer_ObserverServices;
class CCSPlayer_ItemServices;

class CCSPlayerPawn;
class CBasePlayerController;

class CBasePlayerPawn : public CBaseModelEntity {
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerPawn);

	SCHEMA_FIELD(CPlayer_MovementServices*, m_pMovementServices);
	SCHEMA_FIELD_POINTER(CHandle<CBasePlayerController>, m_hController);
	SCHEMA_FIELD(CCSPlayer_ItemServices*, m_pItemServices);
	SCHEMA_FIELD(CPlayer_ObserverServices*, m_pObserverServices);
	SCHEMA_FIELD(CCSPlayer_WaterServices*, m_pWaterServices);

public:
	bool IsBot() {
		return !!(this->m_fFlags() & FL_PAWN_FAKECLIENT);
	}

	void CommitSuicide(bool bExplode, bool bForce);
};