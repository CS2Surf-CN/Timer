#include <surf/replay/surf_replay.h>
#include <core/eventmanager.h>
#include <utils/ctimer.h>

EVENT_CALLBACK_POST(OnPlayerSpawm) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	CHandle<CCSPlayerController> hController = pController->GetRefEHandle();

	UTIL::RequestFrame([hController]() {
		CCSPlayerController* pController = hController.Get();
		if (!pController || !pController->IsBot()) {
			return;
		}

		if (!pController->m_bPawnIsAlive()) {
			return;
		}

		CBasePlayerPawn* pPawn = pController->GetCurrentPawn();
		if (!pPawn || !pPawn->IsAlive()) {
			return;
		}

		pPawn->SetMoveType(MOVETYPE_NOCLIP);
	});
}

EVENT_CALLBACK_PRE(OnPlayerDeath) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return true;
	}

	if (pController->IsBot()) {
		bDontBroadcast = true;
	}

	return true;
}

void CSurfReplayPlugin::HookEvents() {
	EVENT::HookEvent("player_spawn", ::OnPlayerSpawm);
	EVENT::HookEvent("player_death", ::OnPlayerDeath);
}
