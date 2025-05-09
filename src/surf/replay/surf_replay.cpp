#include "surf_replay.h"
#include <utils/print.h>
#include <core/concmdmanager.h>
#include <core/cvarmanager.h>
#include <core/sdkhook.h>

CSurfReplayPlugin g_SurfReplay;

CSurfReplayPlugin* SURF::ReplayPlugin() {
	return &g_SurfReplay;
}

#include <utils/ctimer.h>

void CSurfReplayPlugin::OnPluginStart() {
	m_cvarTrackPreRunTime = CVAR::Register("surf_replay_track_preruntime", 1.5f, "Time (in seconds) to record before a player leaves start zone.", 0.0f, 2.0f);
	m_cvarTrackPostRunTime = CVAR::Register("surf_replay_track_postruntime", 2.0f, "Time (in seconds) to record after a player enters the end zone.", 0.0f, 2.0f);
	m_cvarStagePreRunTime = CVAR::Register("surf_replay_stage_preruntime", 1.5f, "Time (in seconds) to record before a player leaves stage zone.", 0.0f, 2.0f);
	m_cvarStagePostRunTime = CVAR::Register("surf_replay_stage_postruntime", 1.5f, "Time (in seconds) to record after a player finished a stage.", 0.0f, 2.0f);
	m_cvarPreRunAlways = CVAR::Register("surf_replay_prerun_always", true, "Record prerun frames outside the start zone?");
}

void CSurfReplayPlugin::OnEntitySpawned(CEntityInstance* pEntity) {
	const char* sClassname = pEntity->GetClassname();
	if (V_strstr(sClassname, "trigger_") || V_strstr(sClassname, "_door")) {
		SDKHOOK::HookEntity<SDKHookType::SDKHook_StartTouch>((CBaseEntity*)pEntity, SURF::REPLAY::HOOK::OnBotTrigger);
		SDKHOOK::HookEntity<SDKHookType::SDKHook_EndTouch>((CBaseEntity*)pEntity, SURF::REPLAY::HOOK::OnBotTrigger);
		SDKHOOK::HookEntity<SDKHookType::SDKHook_Touch>((CBaseEntity*)pEntity, SURF::REPLAY::HOOK::OnBotTrigger);
		// SDKHOOK::HookEntity<SDKHookType::SDKHook_Use>((CBaseEntity*)pEntity, SURF::REPLAY::HOOK::HookBotTrigger);
	}
}

bool CSurfReplayPlugin::OnPlayerRunCmd(CCSPlayerPawn* pPawn, CInButtonState& buttons, float (&vec)[3], QAngle& viewAngles, int& weapon, int& cmdnum, int& tickcount, int& seed, int (&mouse)[2]) {
	if (pPawn->IsBot()) {
		CSurfBot* pBot = SURF::GetBotManager()->ToPlayer(pPawn);
		if (pBot) {
			pBot->m_pReplayService->DoPlayback(pPawn, buttons);
		}
	}

	return true;
}

void CSurfReplayPlugin::OnPlayerRunCmdPost(CCSPlayerPawn* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pPawn);
	if (!player) {
		return;
	}

	player->m_pReplayService->DoRecord(pPawn, buttons, viewAngles);
}

bool CSurfReplayPlugin::OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) {
	if (zone.m_iType == EZoneType::Zone_Start) {
		pPlayer->m_pReplayService->OnEnterStart_Recording();
	}
	else if (zone.m_iType == EZoneType::Zone_End) {
		pPlayer->m_pReplayService->SaveRecord();
	}

	return true;
}

bool CSurfReplayPlugin::OnStayZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) {
	if (zone.m_iType == EZoneType::Zone_Start) {
		pPlayer->m_pReplayService->OnStart_Recording();
	}

	return true;
}

bool CSurfReplayPlugin::OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) {
	/*if (zone.m_iType == ZoneType::Zone_Start) {
		pPlayer->m_pReplayService->StartRecord();
	}*/

	return true;
}

void CSurfReplayPlugin::OnTimerFinishPost(CSurfPlayer* pPlayer) {
	pPlayer->m_pReplayService->OnTimerFinishPost_SaveRecording();
}

void CSurfReplayService::OnReset() {
	m_bEnabled = false;
	m_vCurrentFrames.resize(10000);
	m_vCurrentFrames.clear();
}
