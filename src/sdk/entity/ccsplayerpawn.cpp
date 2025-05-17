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

CBaseViewModel* CCSPlayerPawnBase::EnsureViewModel(int vmSlot) {
	CBaseViewModel* pCustomViewModel = m_pViewModelServices()->GetViewModel(vmSlot);
	if (!pCustomViewModel) {
		pCustomViewModel = (CBaseViewModel*)MEM::CALL::CreateEntityByName("predicted_viewmodel");
		pCustomViewModel->DispatchSpawn();
		m_pViewModelServices()->SetViewModel(vmSlot, pCustomViewModel);
		pCustomViewModel->m_hOwnerEntity(this->GetRefEHandle());
	} else {
		pCustomViewModel->Teleport(nullptr, &vec3_angle, nullptr);
	}

	return pCustomViewModel;
}
