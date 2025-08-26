#include <core/concmdmanager.h>
#include <surf/surf_player.h>
#include <utils/print.h>
#include <surf/misc/surf_misc.h>
#include <surf/misc/showtrigger.h>
#include <surf/misc/hide.h>

CCMD_CALLBACK(Command_Hide) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return;
	}

	auto& pMiscService = pPlayer->m_pMiscService;
	pMiscService->m_bHide = !pMiscService->m_bHide;

	auto vOnlinePlayers = GetPlayerManager()->GetOnlinePlayers();
	for (const auto& pOnlinePlayer : vOnlinePlayers) {
		CCSPlayerController* pTargetController = pOnlinePlayer->GetController();
		if (pTargetController && pTargetController->GetPlayerSlot() != pController->GetPlayerSlot()) {
			CCSPlayerPawn* pTargetPawn = pTargetController->GetPlayerPawn();
			if (pTargetPawn) {
				SURF::MISC::HidePlugin()->Set(pController, pTargetPawn, pMiscService->m_bHide);
			}
		}
	}

	pPlayer->Print("[其他玩家] %s", pMiscService->m_bHide ? "已隐藏" : "已显示");
}

// FIXME
CCMD_CALLBACK(Command_HideWeapons) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return;
	}

#if 0
	pPlayer->PrintWarning("功能维护中!");
#else
	auto& pMiscService = pPlayer->m_pMiscService;
	pMiscService->m_bHideWeapons = !pMiscService->m_bHideWeapons;
	pMiscService->HideWeapons();

	pPlayer->Print("[武器] %s", pMiscService->m_bHideWeapons ? "已隐藏" : "已显示");
#endif
}

CCMD_CALLBACK(Command_HideLegs) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return;
	}

	auto& pMiscService = pPlayer->m_pMiscService;
	pMiscService->m_bHideLegs = !pMiscService->m_bHideLegs;
	pMiscService->HideLegs();

	pPlayer->Print("[腿部] %s", pMiscService->m_bHideLegs ? "已隐藏" : "已显示");
}

CCMD_CALLBACK(Command_ShowTrigger) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return;
	}

	auto& pMiscService = pPlayer->m_pMiscService;
	pMiscService->m_bShowTrigger = !pMiscService->m_bShowTrigger;

	pPlayer->Print("[显示区域] %s", pMiscService->m_bShowTrigger ? "已打开" : "已关闭");
	if (pMiscService->m_bShowTrigger) {
		pPlayer->Print("请控制台输入 cl_debug_overlays_broadcast 1");
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
