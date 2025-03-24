#include "surf_replay.h"
#include <utils/print.h>
#include <core/concmdmanager.h>

CSurfReplayPlugin g_SurfReplay;

CSurfReplayPlugin* SURF::ReplayPlugin() {
	return &g_SurfReplay;
}

void CSurfReplayPlugin::OnPluginStart() {}

void CSurfReplayPlugin::OnPlayerRunCmdPost(CCSPlayerPawn* pawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	if (!pawn->IsAlive()) {
		return;
	}

	if (pawn->IsBot()) {
		DoPlayback(pawn, pawn->m_pBot());
	} else {
		CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pawn);
		if (!player) {
			return;
		}

		auto& pReplayService = player->m_pReplayService;
		if (pReplayService->m_bEnabled) {
			pReplayService->DoRecord(pawn, buttons, viewAngles);
		}
	}
}

bool CSurfReplayPlugin::OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* player) {
	if (zone.m_iType == ZoneType::Zone_End) {
		player->m_pReplayService->SaveRecord();
	}

	return true;
}

bool CSurfReplayPlugin::OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* player) {
	if (zone.m_iType == ZoneType::Zone_Start) {
		player->m_pReplayService->StartRecord();
	}

	return true;
}

void CSurfReplayService::Reset() {
	m_bEnabled = false;
	m_vReplayFrames.clear();
}
