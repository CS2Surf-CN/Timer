#include "ccsplayerpawn.h"
#include <sdk/entity/services.h>
#include <core/memory.h>

Vector CCSPlayerPawnBase::GetEyePosition() {
	Vector absorigin = GetAbsOrigin();
	CPlayer_CameraServices* cameraService = m_pCameraServices();
	if (!cameraService) {
		return absorigin;
	}
	return Vector(absorigin.x, absorigin.y, absorigin.z + cameraService->m_flOldPlayerViewOffsetZ());
}

QAngle CCSPlayerPawnBase::GetEyeAngle() {
	return !IsObserver() ? static_cast<CCSPlayerPawn*>(this)->m_angEyeAngles() : m_CBodyComponent()->m_pSceneNode()->m_angRotation();
}
