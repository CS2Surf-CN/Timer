#include "services.h"

CBaseViewModel* CCSPlayer_ViewModelServices::GetViewModel(int iIndex) {
	return m_hViewModel()[iIndex].Get();
}

void CCSPlayer_ViewModelServices::SetViewModel(int iIndex, CBaseViewModel* pViewModel) {
	m_hViewModel()[iIndex].Set(pViewModel);
	this->GetPawn()->NetworkStateChanged();
}
