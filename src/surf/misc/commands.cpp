#include <core/concmdmanager.h>
#include <surf/surf_player.h>
#include <utils/print.h>
#include <surf/misc/surf_misc.h>
#include <surf/misc/showtrigger.h>

static void Command_HideLegs(CCSPlayerController* pController, const std::vector<std::string>& vArgs) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto& pMiscService = player->m_pMiscService;
	pMiscService->m_bHideLegs = !pMiscService->m_bHideLegs;

	UTIL::PrintChat(pController, "%s 腿部\n", pMiscService->m_bHideLegs ? "已隐藏" : "已显示");
}

static void Command_ShowTrigger(CCSPlayerController* pController, const std::vector<std::string>& vArgs) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto& pMiscService = player->m_pMiscService;
	pMiscService->m_bShowTrigger = !pMiscService->m_bShowTrigger;

	UTIL::PrintChat(pController, "[显示区域] %s\n", pMiscService->m_bShowTrigger ? "已打开" : "已关闭");

	g_ShowTrigger.TransmitTriggers(SurfMiscPlugin()->m_vTriggers, pMiscService->m_bShowTrigger);
}

void RegisterCommands() {
	CONCMD::RegConsoleCmd("sm_hideleg", Command_HideLegs);
	CONCMD::RegConsoleCmd("sm_hidelegs", Command_HideLegs);
	CONCMD::RegConsoleCmd("sm_st", Command_ShowTrigger);
	CONCMD::RegConsoleCmd("sm_showtrigger", Command_ShowTrigger);
	CONCMD::RegConsoleCmd("sm_showtriggers", Command_ShowTrigger);
}
