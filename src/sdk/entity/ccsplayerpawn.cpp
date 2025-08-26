#include "ccsplayerpawn.h"
#include <sdk/entity/services.h>
#include <core/memory.h>

Vector CCSPlayerPawnBase::GetEyePosition() {
	Vector absorigin = GetAbsOrigin();
	const auto& viewOffset = m_vecViewOffset();
	return absorigin + viewOffset.m_vecData;
}

QAngle CCSPlayerPawnBase::GetEyeAngle() {
	return !IsObserver() ? static_cast<CCSPlayerPawn*>(this)->m_angEyeAngles() : m_CBodyComponent()->m_pSceneNode()->m_angRotation();
}
