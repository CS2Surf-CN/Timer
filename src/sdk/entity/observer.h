#pragma once

#include <sdk/entity/ccsplayerpawn.h>
#include <sdk/entity/services.h>

class CCSObserver_ObserverServices : public CPlayer_ObserverServices {};

class CCSObserver_MovementServices : public CPlayer_MovementServices {};

class CCSObserver_UseServices : public CPlayer_UseServices {};

class CCSObserver_CameraServices : public CCSPlayerBase_CameraServices {};

class CCSObserverPawn : public CCSPlayerPawnBase {
public:
	CCSObserver_ObserverServices* GetObserverServices() {
		static auto iOffset = schema::GetOffset("CCSObserverPawn", "m_pObserverServices");
		return *(CCSObserver_ObserverServices**)((uint8_t*)this + iOffset.offset);
	};

	CCSObserver_MovementServices* GetMovementServices() {
		static auto iOffset = schema::GetOffset("CCSObserverPawn", "m_pMovementServices");
		return *(CCSObserver_MovementServices**)((uint8_t*)this + iOffset.offset);
	};

	CCSObserver_UseServices* GetUseServices() {
		static auto iOffset = schema::GetOffset("CCSObserverPawn", "m_pUseServices");
		return *(CCSObserver_UseServices**)((uint8_t*)this + iOffset.offset);
	};

	CCSObserver_CameraServices* GetCameraServices() {
		static auto iOffset = schema::GetOffset("CCSObserverPawn", "m_pCameraServices");
		return *(CCSObserver_CameraServices**)((uint8_t*)this + iOffset.offset);
	};
};
