#include <core/concmdmanager.h>
#include <surf/surf_player.h>
#include <utils/print.h>
#include <surf/misc/surf_misc.h>
#include <surf/misc/showtrigger.h>
#include <surf/misc/hide.h>

CCMD_CALLBACK(Command_Hide) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto& pMiscService = player->m_pMiscService;
	pMiscService->m_bHide = !pMiscService->m_bHide;

	auto vOnlinePlayers = GetPlayerManager()->GetOnlinePlayers();
	for (const auto& pPlayer : vOnlinePlayers) {
		CCSPlayerController* pTargetController = pPlayer->GetController();
		if (pTargetController && pTargetController->GetPlayerSlot() != pController->GetPlayerSlot()) {
			CCSPlayerPawn* pTargetPawn = pTargetController->GetPlayerPawn();
			if (pTargetPawn) {
				SURF::MISC::HidePlugin()->Set(pController, pTargetPawn, pMiscService->m_bHide);
			}
		}
	}

	UTIL::PrintChat(pController, "[其他玩家] %s\n", pMiscService->m_bHide ? "已隐藏" : "已显示");
}

// FIXME
CCMD_CALLBACK(Command_HideWeapons) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return;
	}

	auto& pMiscService = pPlayer->m_pMiscService;
	pMiscService->m_bHideWeapons = !pMiscService->m_bHideWeapons;

	auto pPawn = pController->GetPlayerPawn();
	auto pWeaponService = pPawn->m_pWeaponServices();
	auto pActiveWeapon = pWeaponService->m_hActiveWeapon().Get();
	if (pActiveWeapon) {
		pWeaponService->m_hActiveWeapon().Set(nullptr);
		pPlayer->Print("[武器] %s\n", pMiscService->m_bHideWeapons ? "已隐藏" : "已显示");	
	} else {
	
	}
}

CCMD_CALLBACK(Command_HideLegs) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto& pMiscService = player->m_pMiscService;
	pMiscService->m_bHideLegs = !pMiscService->m_bHideLegs;
	pMiscService->HideLegs();

	UTIL::PrintChat(pController, "[腿部] %s\n", pMiscService->m_bHideLegs ? "已隐藏" : "已显示");
}

CCMD_CALLBACK(Command_ShowTrigger) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto& pMiscService = player->m_pMiscService;
	pMiscService->m_bShowTrigger = !pMiscService->m_bShowTrigger;

	UTIL::PrintChat(pController, "[显示区域] %s\n", pMiscService->m_bShowTrigger ? "已打开" : "已关闭");
	if (pMiscService->m_bShowTrigger) {
		UTIL::PrintChat(pController, "请控制台输入 cl_debug_overlays_broadcast 1\n");
	}

	SURF::MISC::ShowTriggerPlugin()->TransmitTriggers(SURF::MiscPlugin()->m_vTriggers, pMiscService->m_bShowTrigger);
}

void CSurfMiscPlugin::RegisterCommands() {
	CONCMD::RegConsoleCmd("sm_hide", Command_Hide);
	CONCMD::RegConsoleCmd("sm_hw", Command_HideWeapons);
	CONCMD::RegConsoleCmd("sm_hideweapon", Command_HideWeapons);
	CONCMD::RegConsoleCmd("sm_hideweapons", Command_HideWeapons);
	CONCMD::RegConsoleCmd("sm_hl", Command_HideLegs);
	CONCMD::RegConsoleCmd("sm_hideleg", Command_HideLegs);
	CONCMD::RegConsoleCmd("sm_hidelegs", Command_HideLegs);
	CONCMD::RegConsoleCmd("sm_st", Command_ShowTrigger);
	CONCMD::RegConsoleCmd("sm_showtrigger", Command_ShowTrigger);
	CONCMD::RegConsoleCmd("sm_showtriggers", Command_ShowTrigger);
}
