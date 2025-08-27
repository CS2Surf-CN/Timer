#include <surf/checkpoint/surf_checkpoint.h>
#include <surf/surf_player.h>
#include <core/concmdmanager.h>
#include <utils/utils.h>
#include <surf/misc/surf_misc.h>

CCMD_CALLBACK(Command_Checkpoints) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;
#if 0
	pCPService->OpenCheckpointsMenu();
#else
	pCPService->Print("请打开无线电菜单 (默认按z呼出再按9)");
#endif
}

CCMD_CALLBACK(Command_Save) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;
	pCPService->SaveCheckpoint();
	pCPService->m_iCurrentCP = pCPService->GetLatestCheckpoint();
}

CCMD_CALLBACK(Command_Tele) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;
	i32 iCP = pCPService->m_iCurrentCP;
	if (vArgs.size() > 0) {
		std::string sCP = vArgs.at(0);
		UTIL::ReplaceString(sCP, "#", "");
		iCP = V_StringToInt32(sCP.c_str(), -1);

		if (iCP == -1) {
			pCPService->Print("输入格式不合法!");
			return;
		}
	}

	pCPService->LoadCheckpoint(iCP);
}

CCMD_CALLBACK(Command_PrevCP) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;
	pCPService->LoadPrev();
}

CCMD_CALLBACK(Command_NextCP) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;
	pCPService->LoadNext();
}

CCMD_CALLBACK(Command_DeleteCP) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;
	pCPService->DeleteCurrentCheckpoint();
}

CCMD_CALLBACK(Command_ResetCP) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;
	pCPService->ResetCheckpoint();
}

CCMDLISTENER_CALLBACK(Command_Jointeam) {
	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return true;
	}

	auto& pCPService = pSurfPlayer->m_pCheckpointService;

	return true;
}

void CSurfCheckpointPlugin::RegisterCommands() {
	CONCMD::RegConsoleCmd("sm_cp", Command_Checkpoints, "Opens the checkpoints menu.");
	CONCMD::RegConsoleCmd("sm_cps", Command_Checkpoints, "Opens the checkpoints menu. Alias for sm_cpmenu.");
	CONCMD::RegConsoleCmd("sm_checkpoint", Command_Checkpoints, "Opens the checkpoints menu. Alias for sm_cpmenu.");
	CONCMD::RegConsoleCmd("sm_checkpoints", Command_Checkpoints, "Opens the checkpoints menu. Alias for sm_cpmenu.");
	CONCMD::RegConsoleCmd("sm_save", Command_Save, "Saves checkpoint.");
	CONCMD::RegConsoleCmd("sm_saveloc", Command_Save, "Saves checkpoint.");
	CONCMD::RegConsoleCmd("sm_tele", Command_Tele, "Teleports to checkpoint. Usage: sm_tele [number]");
	CONCMD::RegConsoleCmd("sm_prac", Command_Tele, "Teleports to checkpoint. Usage: sm_tele [number]. Alias of sm_tele.");
	CONCMD::RegConsoleCmd("sm_practice", Command_Tele, "Teleports to checkpoint. Usage: sm_tele [number]. Alias of sm_tele.");
	CONCMD::RegConsoleCmd("sm_prev", Command_PrevCP, "Teleport to previous checkpoint.");
	CONCMD::RegConsoleCmd("sm_next", Command_NextCP, "Teleport to next checkpoint.");
	CONCMD::RegConsoleCmd("sm_delcp", Command_DeleteCP, "Delete current checkpoint.");
	CONCMD::RegConsoleCmd("sm_resetcp", Command_ResetCP, "Reset checkpoint.");

	CONCMD::AddCommandListener("jointeam", Command_Jointeam);
}
