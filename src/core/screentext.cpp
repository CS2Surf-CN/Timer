#include "screentext.h"
#include <core/memory.h>
#include <core/eventmanager.h>
#include <core/concmdmanager.h>
#include <utils/ctimer.h>
#include <utils/utils.h>

#define VIEWMODEL_BROKEN 1

CScreenTextControllerManager g_ScreenTextControllerManager;

void SetScreenTextEntityTransmiter(CBaseEntity* pScreenEnt, CBasePlayerController* pOwner);

CPointWorldText* CScreenText::EnsureScreenEntity() {
	auto pScreenEnt = m_hScreenEnt.Get();
	if (pScreenEnt) {
		return pScreenEnt;
	}

	CPointWorldText* pText = static_cast<CPointWorldText*>(MEM::CALL::CreateEntityByName("point_worldtext"));
	if (!pText) {
		Assert(false);
		return nullptr;
	}

	pText->m_Color(m_Manifest.m_Color);
	pText->m_FontName(m_Manifest.m_sFont.c_str());
	pText->m_flFontSize(m_Manifest.m_fFontSize);
	pText->m_flWorldUnitsPerPx(GetWorldUnits(m_Manifest.m_iUnits));
	pText->m_flDepthOffset(0.0f);
	pText->m_fadeMinDist(0.0f);
	pText->m_nJustifyHorizontal(PointWorldTextJustifyHorizontal_t::POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_LEFT);
	pText->m_nJustifyVertical(PointWorldTextJustifyVertical_t::POINT_WORLD_TEXT_JUSTIFY_VERTICAL_CENTER);
	pText->m_bFullbright(true);
	pText->m_bDrawBackground(m_Manifest.m_bBackground);
	pText->m_flBackgroundBorderHeight(m_Manifest.m_fBackgroundBorderHeight);
	pText->m_flBackgroundBorderWidth(m_Manifest.m_fBackgroundBorderWidth);
	pText->m_messageText(m_Manifest.m_sText.c_str());
	pText->m_bEnabled(m_Manifest.m_bEnable);

	pText->DispatchSpawn();

	m_hScreenEnt = pText->GetRefEHandle();

	return pText;
}

CScreenText::CScreenText(const ScreenTextManifest_t& manifest) {
	m_Manifest = manifest;
	EnsureScreenEntity();
}

CScreenText::~CScreenText() {
	if (m_hScreenEnt.IsValid()) {
		auto pScreenEnt = m_hScreenEnt.Get();
		if (pScreenEnt) {
			pScreenEnt->Kill();
		}

		m_hScreenEnt.Term();
	}

	m_hOwner.Term();
	m_hOriginalController.Term();
}

void CScreenText::SetText(const std::string_view& text) {
	auto pScreenEnt = m_hScreenEnt.Get();
	if (pScreenEnt) {
		m_Manifest.m_sText = text;
		pScreenEnt->SetText(text.data());
	}
}

void CScreenText::SetPos(float x, float y) {
	m_Manifest.m_vecPos.x = x;
	m_Manifest.m_vecPos.y = y;

	UpdatePos();
}

void CScreenText::SetColor(Color color) {
	auto pScreenEnt = m_hScreenEnt.Get();
	if (pScreenEnt) {
		m_Manifest.m_Color = color;
		pScreenEnt->m_Color(color);
	}
}

void CScreenText::SetFont(const std::string_view& font) {
	auto pScreenEnt = m_hScreenEnt.Get();
	if (pScreenEnt) {
		m_Manifest.m_sFont = font;
		pScreenEnt->m_FontName(font.data());
	}
}

void CScreenText::SetFontSize(float fontsize) {
	auto pScreenEnt = m_hScreenEnt.Get();
	if (pScreenEnt) {
		m_Manifest.m_fFontSize = fontsize;
		pScreenEnt->m_flFontSize(fontsize);
	}
}

void CScreenText::SetUnits(int unit) {
	auto pScreenEnt = m_hScreenEnt.Get();
	if (pScreenEnt) {
		m_Manifest.m_iUnits = unit;
		pScreenEnt->m_flWorldUnitsPerPx(GetWorldUnits(unit));
	}
}

bool CScreenText::IsRendering() {
	auto pScreenEnt = m_hScreenEnt.Get();
	if (pScreenEnt) {
		return m_hOwner.IsValid() && pScreenEnt->m_bEnabled();
	}

	return false;
}

void CScreenText::Display(CBasePlayerController* pController) {
	// Already displayed
	if (IsRendering()) {
		return;
	}

	EnsureScreenEntity();

	CPointWorldText* pText = this->m_hScreenEnt.Get();
	if (!pText) {
		Assert(false);
		return;
	}

	CCSPlayerPawnBase* pPawn = dynamic_cast<CCSPlayerPawnBase*>(pController->GetCurrentPawn());
	if (!pPawn) {
		Assert(false);
		return;
	}

	pText->Enable();

	UpdateTransmit(pController);
	UpdateRelation(pPawn);
	UpdatePos();
}

void CScreenText::UpdatePos(CCSPlayerPawnBase* pPawn) {
	auto pText = m_hScreenEnt.Get();

	if (!pText) {
		return;
	}

	Vector forward;
	Vector right;
	Vector up;
	const auto& parentAng = pPawn->GetEyeAngle();
	AngleVectors(parentAng, &forward, &right, &up);
	Vector textPos = pPawn->GetEyePosition();
	textPos += (forward * 7.10f);
	// -x = move left,  +x = move right
	textPos += (right * m_Manifest.m_vecPos.x);

	// -y = move up,   +y = move down
	textPos -= (up * m_Manifest.m_vecPos.y);

	QAngle textAng;
	textAng.x = 0.f;
	textAng.y = AngleNormalize(parentAng.y - 90.0f);
	textAng.z = AngleNormalize(-parentAng.x + 90.0f);

	pText->Teleport(&textPos, &textAng, nullptr);
}

void CScreenText::UpdateRelation(CCSPlayerPawnBase* pPawn) {
	if (!pPawn) {
		return;
	}

	CPointWorldText* pText = this->m_hScreenEnt.Get();
	if (!pText) {
		Assert(false);
		return;
	}

#ifndef VIEWMODEL_BROKEN
	if (!pPawn->IsObserver()) {
		CBaseViewModel* pViewModel = pPawn->GetCustomViewModel();
		if (!pViewModel) {
			Assert(false);
			return;
		}

		pText->SetParent(pViewModel);
		pText->m_hOwnerEntity(pViewModel);
	} else {
		CBaseEntity* pTarget = pPawn;

		CPlayer_ObserverServices* pObsService = pPawn->m_pObserverServices();
		CCSPlayerPawnBase* pObsTarget = dynamic_cast<CCSPlayerPawnBase*>(pObsService->m_hObserverTarget()->Get());
		if (pObsTarget && pObsService->m_iObserverMode() == OBS_MODE_IN_EYE) {
			pTarget = pObsTarget->GetCustomViewModel();
		}

		pText->SetParent(pTarget);
		pText->m_hOwnerEntity(pTarget);
	}
#else
	pText->SetParent(pPawn);
	pText->m_hOwnerEntity(pPawn);
#endif

	m_hOwner = pPawn->GetRefEHandle();
}

void CScreenText::UpdatePos() {
	auto pText = m_hScreenEnt.Get();

	if (!pText || !pText->m_hOwnerEntity().IsValid()) {
		return;
	}

	CBaseEntity* pParent = pText->m_hOwnerEntity().Get();
#ifndef VIEWMODEL_BROKEN
	const Vector& parentPos = pParent->GetAbsOrigin();
	Vector textPos;
	if (dynamic_cast<CBaseViewModel*>(pParent)) {
		textPos = GetRelativeVMOrigin(parentPos);
	} else {
		static QAngle nullAng;
		pParent->Teleport(nullptr, &nullAng, nullptr);
		textPos = GetRelativePawnOrigin(parentPos, nullAng);
	}

	static QAngle textAng = {0.0f, -90.0f, 90.0f};

	Vector fwd, right;
	AngleVectors(textAng, &fwd, &right, nullptr);
	fwd *= m_Manifest.m_vecPos.x;
	right *= m_Manifest.m_vecPos.y * -1.0f;
	textPos += fwd + right;
#else
	const auto& parentPos = static_cast<CCSPlayerPawnBase*>(pParent)->GetEyePosition();
	const auto& parentAng = static_cast<CCSPlayerPawnBase*>(pParent)->GetEyeAngle();
	Vector textPos = parentPos;

	Vector forward;
	Vector right;
	Vector up;
	AngleVectors(parentAng, &forward, &right, &up);
	textPos += (forward * 7.10f);
	// -x = move left,  +x = move right
	textPos += (right * m_Manifest.m_vecPos.x);

	// -y = move up,   +y = move down
	textPos -= (up * m_Manifest.m_vecPos.y);

	QAngle textAng;
	textAng.x = 0.f;
	textAng.y = AngleNormalize(parentAng.y - 90.0f);
	textAng.z = AngleNormalize(-parentAng.x + 90.0f);
#endif

	pText->Teleport(&textPos, &textAng, nullptr);
}

void CScreenText::UpdateTransmit(CBasePlayerController* pOwner) {
	CPointWorldText* pText = this->m_hScreenEnt.Get();
	if (!pText) {
		Assert(false);
		return;
	}

	SetScreenTextEntityTransmiter(pText, pOwner);
}

Vector CScreenText::GetRelativeVMOrigin(const Vector& eyePosition, float distanceToTarget) {
	return Vector(eyePosition.x + distanceToTarget, eyePosition.y, eyePosition.z);
}

Vector CScreenText::GetRelativePawnOrigin(const Vector& eyePosition, const QAngle& eyeAngles, float distanceToTarget) {
	double pitch = eyeAngles.x * (M_PI / 180.0);
	double yaw = eyeAngles.y * (M_PI / 180.0);

	double targetX = eyePosition.x + distanceToTarget * std::cos(pitch) * std::cos(yaw);
	double targetY = eyePosition.y + distanceToTarget * std::cos(pitch) * std::sin(yaw);
	double targetZ = eyePosition.z - distanceToTarget * std::sin(pitch);

	return Vector(targetX, targetY, targetZ);
};

CScreenTextControllerManager* VGUI::GetScreenTextManager() {
	return &g_ScreenTextControllerManager;
}

std::weak_ptr<CScreenText> VGUI::CreateScreenText(CBasePlayerController* pController, std::optional<ScreenTextManifest_t> manifest) {
	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		Assert(false);
		return {};
	}

	if (!manifest) {
		manifest = ScreenTextManifest_t {};
	}

	auto pText = std::make_shared<CScreenText>(manifest.value());
	pText->m_hOriginalController = pController->GetRefEHandle();
	pTextController->m_ScreenTextList.emplace_back(pText);
	return pText;
}

void VGUI::Render(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (pText->IsRendering()) {
		return;
	}

	if (!pText->m_hOriginalController.IsValid()) {
		return;
	}

	auto pController = pText->m_hOriginalController.Get();
	if (!pController) {
		return;
	}

	pText->Display(pController);
}

void VGUI::Unrender(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (!pText->IsRendering()) {
		return;
	}

	pText->Disable();
}

void VGUI::Dispose(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (!pText->m_hOriginalController.IsValid()) {
		return;
	}

	auto pController = pText->m_hOriginalController.Get();
	if (!pController) {
		return;
	}

	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		Assert(false);
		return;
	}

	std::erase(pTextController->m_ScreenTextList, pText);
}

void VGUI::Cleanup(CBasePlayerController* pController) {
	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		Assert(false);
		return;
	}

	pTextController->m_ScreenTextList.clear();
}

void CScreenTextControllerManager::OnPluginStart() {
	EVENT::HookEvent("player_team", OnPlayerTeam);
	EVENT::HookEvent("cs_intermission", OnIntermission);
}

void CScreenTextControllerManager::OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	CScreenTextController* pTextController = VGUI::GetScreenTextManager()->ToPlayer(pPawn);
	if (!pTextController) {
		return;
	}

	for (const auto& pScreenText : pTextController->m_ScreenTextList) {
		pScreenText->UpdatePos(pPawn);
	}
}

void CScreenTextControllerManager::OnSetObserverTargetPost(CPlayer_ObserverServices* pService, CBaseEntity* pEnt, const ObserverMode_t iObsMode) {
	if (iObsMode == OBS_MODE_IN_EYE || iObsMode == OBS_MODE_CHASE) {
		auto pServicePawn = pService->GetPawn();
		auto pPlayer = VGUI::GetScreenTextManager()->ToPlayer(pServicePawn);
		if (!pPlayer) {
			return;
		}

		for (const auto& pScreenText : pPlayer->m_ScreenTextList) {
			pScreenText->UpdateRelation(pServicePawn);
			if (pEnt->IsPawn()) {
				pScreenText->UpdatePos(static_cast<CCSPlayerPawn*>(pEnt));
			}
		}
	}
}

void CScreenTextControllerManager::OnPlayerTeam(IGameEvent* pEvent, const char* szName, bool bServerOnly) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	auto iNewTeam = pEvent->GetInt("team");
	auto hController = pController->GetRefEHandle();

	UTIL::RequestFrame([hController, iNewTeam]() {
		CCSPlayerController* pController = static_cast<CCSPlayerController*>(hController.Get());
		if (!pController) {
			return;
		}

		CCSPlayerPawnBase* pPawn = dynamic_cast<CCSPlayerPawnBase*>(pController->GetCurrentPawn());
		if (!pPawn) {
			return;
		}

		auto pPlayer = VGUI::GetScreenTextManager()->ToPlayer(pController);
		if (!pPlayer) {
			return;
		}

		for (const auto& pScreenText : pPlayer->m_ScreenTextList) {
			pScreenText->UpdateRelation(pPawn);
			pScreenText->UpdatePos(pPawn);
		}
	});
}

void CScreenTextControllerManager::OnIntermission(IGameEvent* pEvent, const char* szName, bool bServerOnly) {
	auto iMaxPlayers = UTIL::GetGlobals()->maxClients;
	for (auto i = 0; i < iMaxPlayers; i++) {
		auto pTextController = VGUI::GetScreenTextManager()->ToPlayer(CPlayerSlot(i));
		if (!pTextController || pTextController->IsFakeClient()) {
			return;
		}

		// FIXME: Rerender on intermission end???
		pTextController->m_ScreenTextList.clear();
	}
}
