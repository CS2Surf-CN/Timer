#include <core/eventmanager.h>
#include <core/interfaces.h>
#include <sdk/gamerules.h>
#include <utils/utils.h>
#include <sdk/entity/ccsplayercontroller.h>
#include <surf/misc/surf_misc.h>

EVENT_CALLBACK_POST(OnPlayerDeath) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	UTIL::RequestFrame([hController = pController->GetRefEHandle()]() {
		CCSPlayerController* pController = static_cast<CCSPlayerController*>(hController.Get());
		if (!pController) {
			return;
		}

		CBasePlayerPawn* pPawn = pController->GetCurrentPawn();
		if (!pPawn || pPawn->IsAlive()) {
			return;
		}

		pPawn->m_clrRender(Color(255, 255, 255, 0));
	});
}

EVENT_CALLBACK_POST(OnRoundPrestart) {}

EVENT_CALLBACK_POST(OnRoundStart) {
	IFACE::pEngine->ServerCommand("sv_full_alltalk 1");

	CCSGameRules* gameRules = UTIL::GetGameRules();
	if (gameRules) {
		gameRules->m_bGameRestart(true);
		gameRules->m_iRoundWinStatus(1);
		// Make sure that the round time is synchronized with the global time.
		gameRules->m_fRoundStartTime().SetTime(0.0f);
		gameRules->m_flGameStartTime().SetTime(0.0f);
	}
}

EVENT_CALLBACK_POST(OnPlayerTeam) {
	pEvent->SetBool("silent", true);
}

EVENT_CALLBACK_POST(OnPlayerSpawm) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	UTIL::RequestFrame([hController = pController->GetRefEHandle()]() {
		CCSPlayerController* pController = static_cast<CCSPlayerController*>(hController.Get());
		if (!pController) {
			return;
		}

		if (!pController->m_bPawnIsAlive()) {
			return;
		}

		CBasePlayerPawn* pPawn = pController->GetCurrentPawn();
		if (!pPawn || !pPawn->IsAlive()) {
			return;
		}

		pPawn->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
		pPawn->m_clrRender(Color(255, 255, 255, 255));

		CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
		if (!pPlayer) {
			return;
		}

		if (pPlayer->m_pMiscService->m_bHideLegs) {
			pPlayer->m_pMiscService->HideLegs();
		}
	});
}

void CSurfMiscPlugin::HookEvents() {
	EVENT::HookEvent("player_death", ::OnPlayerDeath);
	EVENT::HookEvent("round_prestart", ::OnRoundPrestart);
	EVENT::HookEvent("round_start", ::OnRoundStart);
	EVENT::HookEvent("player_team", ::OnPlayerTeam);
	EVENT::HookEvent("player_spawn", ::OnPlayerSpawm);
}
