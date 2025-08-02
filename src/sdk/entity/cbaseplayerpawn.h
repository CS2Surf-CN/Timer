#pragma once

#include <sdk/entity/cbaseentity.h>

class CCSPlayer_WaterServices;
class CPlayer_MovementServices;
class CPlayer_ObserverServices;
class CCSPlayer_ItemServices;
class CPlayer_CameraServices;

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
	SCHEMA_FIELD(CPlayer_CameraServices*, m_pCameraServices);

public:
	CBasePlayerController* GetController() {
		return m_hController()->Get();
	}

	bool IsBot() {
		return !!(this->m_fFlags() & FL_BOT);
	}

	bool IsObserver() {
		return m_pObserverServices() != nullptr;
	}

	void CommitSuicide(bool bExplode, bool bForce);

	void SetCollisionGroup(StandardCollisionGroups_t group);
};
