#include <utils/utils.h>
#include <core/sdkhook.h>

#include <surf/checkpoint/surf_checkpoint.h>
#include <surf/global/surf_global.h>
#include <surf/hud/surf_hud.h>
#include <surf/misc/surf_misc.h>
#include <surf/replay/surf_replay.h>
#include <surf/timer/surf_timer.h>
#include <surf/zones/surf_zones.h>

CSurfPlayerManager g_SurfPlayerManager;

CSurfPlayerManager* SURF::GetPlayerManager() {
	return &g_SurfPlayerManager;
}

CMovementPlayerManager* MOVEMENT::GetPlayerManager() {
	return static_cast<CMovementPlayerManager*>(&g_SurfPlayerManager);
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CServerSideClientBase* pClient) const {
	return static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(pClient));
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CPlayerPawnComponent* component) const {
	return static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(component));
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CBasePlayerController* controller) const {
	return static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(controller));
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CBasePlayerPawn* pawn) const {
	return static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(pawn));
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CPlayerSlot slot) const {
	auto pPlayer = static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(slot));
	return pPlayer->m_bFakeClient ? nullptr : pPlayer;
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CEntityIndex entIndex) const {
	return static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(entIndex));
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CPlayerUserId userID) const {
	return static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(userID));
}

CSurfPlayer* CSurfPlayerManager::ToPlayer(CSteamID steamid, bool validate) const {
	return static_cast<CSurfPlayer*>(CPlayerManager::ToPlayer(steamid, validate));
}

std::vector<CPlayer*> CSurfPlayerManager::GetOnlinePlayers() const {
	std::vector<CPlayer*> players;
	for (auto& player : m_pPlayers) {
		if (!player->m_bFakeClient && UTIL::IsPlayerSlot(player->GetPlayerSlot())) {
			players.emplace_back(player.get());
		}
	}

	return players;
}

static bool Hook_OnPlayerTeleport(CBaseEntity* pSelf, Vector* newPosition, QAngle* newAngles, Vector* newVelocity) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer((CBasePlayerPawn*)pSelf);
	if (!pPlayer) {
		return true;
	}

	pPlayer->m_bJustTeleported = true;

	return true;
}

static void Hook_OnPlayerTeleportPost(CBaseEntity* pSelf, Vector* newPosition, QAngle* newAngles, Vector* newVelocity) {
	auto hPlayerHandle = pSelf->GetRefEHandle();
	UTIL::RequestFrame([hPlayerHandle] {
		UTIL::RequestFrame([hPlayerHandle] {
			CBasePlayerPawn* pPawn = (CBasePlayerPawn*)hPlayerHandle.Get();
			if (pPawn) {
				CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pPawn);
				if (pPlayer) {
					pPlayer->m_bJustTeleported = false;
				}
			}
		});
	});
}

void CSurfPlayerManager::OnEntitySpawned(CEntityInstance* pEntity, bool bMapStarted) {
	auto pszClassname = pEntity->GetClassname();
	if (!V_stricmp(pszClassname, "player")) {
		SDKHOOK::HookEntity<SDKHook_Teleport>((CBaseEntity*)pEntity, Hook_OnPlayerTeleport);
		SDKHOOK::HookEntity<SDKHook_Teleport>((CBaseEntity*)pEntity, Hook_OnPlayerTeleportPost);
	}
}

void CSurfPlayer::Init(int iSlot) {
	CMovementPlayer::Init(iSlot);

	InitService(m_pTimerService);
	InitService(m_pZoneService);
	InitService(m_pHudService);
	InitService(m_pReplayService);
	InitService(m_pMiscService);
	InitService(m_pGlobalAPIService);
	InitService(m_pCheckpointService);

	m_bJustTeleported = false;
}

void CSurfPlayer::Reset() {
	CMovementPlayer::Reset();

	ResetService(m_pTimerService);
	ResetService(m_pZoneService);
	ResetService(m_pHudService);
	ResetService(m_pReplayService);
	ResetService(m_pMiscService);
	ResetService(m_pGlobalAPIService);
	ResetService(m_pCheckpointService);

	m_bJustTeleported = false;
}

void CSurfPlayer::Print(const char* fmt, ...) const {
	auto pController = GetController();
	if (pController) {
		CUtlString buffer;
		va_list args;
		va_start(args, fmt);
		buffer.FormatV(fmt, args);

		UTIL::CPrintChat(pController, "{green}[Surf] {grey}%s", buffer.Get());
	}
}

void CSurfPlayer::PrintWarning(const char* fmt, ...) const {
	auto pController = GetController();
	if (pController) {
		CUtlString buffer;
		va_list args;
		va_start(args, fmt);
		buffer.FormatV(fmt, args);

		UTIL::CPrintChat(pController, "{green}[Surf] {darkred}%s", buffer.Get());
	}
}

TimerStyle_t CSurfPlayer::GetStyle() const {
	return m_pTimerService->m_iCurrentStyle;
}

TimerTrack_t CSurfPlayer::GetCurrentTrack() const {
	return m_pTimerService->m_iCurrentTrack;
}

TimerStage_t CSurfPlayer::GetCurrentStage() const {
	return m_pTimerService->m_iCurrentStage;
}

TimerTime_t CSurfPlayer::GetCurrentTime() const {
	return m_pTimerService->m_fCurrentTime;
}

bool CSurfPlayer::IsPracticeMode() const {
	return m_pTimerService->m_bPracticeMode;
}

bool CSurfPlayer::IsStageTimer() const {
	return m_pTimerService->m_bStageTimer;
}

void CSurfPlayerService::PlayErrorSound() const {
	UTIL::PlaySoundToClient(GetPlayer()->GetPlayerSlot(), SURF_SND_ERROR);
}

void CSurfPlayerService::Print(const char* fmt, ...) const {
	auto pController = GetPlayer()->GetController();
	if (pController) {
		CUtlString buffer;
		va_list args;
		va_start(args, fmt);
		buffer.FormatV(fmt, args);

		UTIL::CPrintChat(pController, "{green}[Surf] {grey}%s", buffer.Get());
	}
}

void CSurfPlayerService::PrintWarning(const char* fmt, ...) const {
	auto pController = GetPlayer()->GetController();
	if (pController) {
		CUtlString buffer;
		va_list args;
		va_start(args, fmt);
		buffer.FormatV(fmt, args);

		UTIL::CPrintChat(pController, "{green}[Surf] {darkred}%s", buffer.Get());
	}
}
