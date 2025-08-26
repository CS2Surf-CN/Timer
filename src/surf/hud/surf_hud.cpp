#include "surf_hud.h"
#include <surf/timer/surf_timer.h>
#include <utils/print.h>
#include <utils/utils.h>
#include <core/eventmanager.h>
#include <fmt/format.h>

CSurfHudPlugin g_SurfHudPlugin;

CSurfHudPlugin* SURF::HudPlugin() {
	return &g_SurfHudPlugin;
}

void CSurfHudPlugin::OnPlayerSpawn(IGameEvent* pEvent, const char* szName, bool bDontBroadcast) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

#if 0
	auto& wpSpeedText = pSurfPlayer->m_pHudService->m_wpSpeedText;
	if (wpSpeedText.expired()) {
		wpSpeedText = VGUI::CreateScreenText(pController);
	}

	if (auto pSpeedText = wpSpeedText.lock()) {
		pSpeedText->SetPos(-0.6f, 3.0f);
		pSpeedText->SetFontSize(40.0f);
	}

	CHandle<CCSPlayerController> hController = pController->GetRefEHandle();
	UTIL::RequestFrame([hController, wpSpeedText]() {
		CCSPlayerController* pController = hController.Get();
		if (!pController || pController->IsBot()) {
			return;
		}

		if (!pController->m_bPawnIsAlive()) {
			return;
		}

		CBasePlayerPawn* pPawn = pController->GetCurrentPawn();
		if (!pPawn || !pPawn->IsAlive()) {
			return;
		}

		VGUI::Render(wpSpeedText);
	});
#endif
}

void CSurfHudPlugin::OnPluginStart() {
	EVENT::HookEvent("player_spawn", OnPlayerSpawn);
}

void CSurfHudPlugin::OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pPawn);
	if (!pSurfPlayer) {
		return;
	}

	if (UTIL::GetGlobals()->tickcount % 2 == 0) {
		return;
	}

	UpdateHudData(pSurfPlayer);
	UpdateHTML(pSurfPlayer);
	//UpdateScreenText(pSurfPlayer);
	UpdateHudDataPost(pSurfPlayer);
}

void CSurfHudPlugin::UpdateHudData(CSurfPlayer* pSurfPlayer) {
	auto& pHudService = pSurfPlayer->m_pHudService;

	Vector vVel;
	pSurfPlayer->GetVelocity(vVel);
	pHudService->m_iCurrentSpeed = std::round(vVel.Length2D());
}

void CSurfHudPlugin::UpdateHudDataPost(CSurfPlayer* pSurfPlayer) {
	auto& pHudService = pSurfPlayer->m_pHudService;

	pHudService->m_iPrevousSpeed = pHudService->m_iCurrentSpeed;
}

void CSurfHudPlugin::UpdateHTML(CSurfPlayer* pSurfPlayer) {
	auto pController = pSurfPlayer->GetController();
	if (!pController) {
		return;
	}

	auto& pTimerService = pSurfPlayer->m_pTimerService;

	CUtlString sTime = SURF::FormatTime(pTimerService->m_fCurrentTime);
	Vector vel;
	pSurfPlayer->GetVelocity(vel);

	// clang-format off
	UTIL::PrintHTMLCenter(pController, " %s时间: %s<br>速度: %.3f%s", 
		"<span class='fontSize-l'>", 
		sTime.Get(),
		vel.Length2D(),
		"</span>");
	// clang-format on
}

void CSurfHudPlugin::UpdateScreenText(CSurfPlayer* pSurfPlayer) {
	auto& pHudService = pSurfPlayer->m_pHudService;

	if (auto pSpeedText = pHudService->m_wpSpeedText.lock()) {
		int iVel = pHudService->m_iCurrentSpeed;

		Color iSpeedColor = iVel < pHudService->m_iPrevousSpeed ? Color(255, 0, 0, 255) : Color(0, 0, 255, 255);
		pSpeedText->SetColor(iSpeedColor);

		auto sAlign = iVel < 100 ? "  " : iVel < 1000 ? " " : "";
		std::string sSpeed = fmt::format("{}{}", sAlign, iVel);
		pSpeedText->SetText(sSpeed);
	}
}

void CSurfHudService::OnReset() {}
