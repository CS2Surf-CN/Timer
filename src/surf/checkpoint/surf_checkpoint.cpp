#include "surf_checkpoint.h"
#include <core/eventmanager.h>
#include <surf/misc/surf_misc.h>

CSurfCheckpointPlugin g_SurfCheckpoint;

CSurfCheckpointPlugin* SURF::CheckpointPlugin() {
	return &g_SurfCheckpoint;
}

void CSurfCheckpointPlugin::OnPluginStart() {
	EVENT::HookEvent("player_spawn", OnPlayerSpawn);
}

void CSurfCheckpointPlugin::OnClientPutInServer(ISource2GameClients* pClient, CPlayerSlot slot, char const* pszName, int type, uint64 xuid) {}

void CSurfCheckpointPlugin::OnClientDisconnect(ISource2GameClients* pClient, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) {}

void CSurfCheckpointPlugin::OnPlayerSpawn(IGameEvent* pEvent, const char* szName, bool bDontBroadcast) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	CSurfPlayer* pSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pSurfPlayer) {
		return;
	}
}

void CSurfCheckpointService::OnReset() {
	// TODO: persist data
	m_vCheckpoints.clear();
	m_iCurrentCP = 0;
}

void CSurfCheckpointService::SaveCheckpoint() {
	CSurfPlayer* pSurfPlayer = this->GetPlayer();
	if (!pSurfPlayer) {
		return;
	}

	CCSPlayerController* pController = pSurfPlayer->GetController();
	if (!pController) {
		return;
	}

	CCSPlayerController* pTargetController = (CCSPlayerController*)pController->GetObserverTarget();
	if (!pTargetController || !pTargetController->IsController()) {
		SURF::Print(pController, "暂不支持观察对象为非玩家的存点.");
		return;
	}

	CCSPlayerPawn* pTargetPawn = pTargetController->GetPlayerPawn();
	if (!pTargetPawn || !pTargetPawn->IsPawn()) {
		SURF::Print(pController, "暂不支持观察对象为非玩家的存点.");
		return;
	}

	CSurfPlayer* pTargetSurfPlayer = SURF::GetPlayerManager()->ToPlayer(pTargetController);
	if (!pTargetSurfPlayer) {
		SURF::Print(pController, "观察对象有问题.");
		return;
	}

	CCSPlayer_MovementServices* pTargetMoveService = pTargetSurfPlayer->GetMoveServices();
	if (!pTargetMoveService) {
		SURF::Print(pController, "观察对象 MoveService 有问题.");
		return;
	}

	cp_cache_t cache;
	pTargetSurfPlayer->GetOrigin(cache.m_vecPos);
	pTargetSurfPlayer->GetAngles(cache.m_vecAng);
	pTargetSurfPlayer->GetVelocity(cache.m_vecVel);
	cache.m_nMoveType = pTargetPawn->m_MoveType();
	cache.m_nActualMoveType = pTargetPawn->m_nActualMoveType();
	cache.m_fGravity = pTargetPawn->m_flGravityScale();
	cache.m_fSpeed = pTargetPawn->m_flVelocityModifier();
	cache.m_iFlags = pTargetPawn->m_fFlags() & ~(FL_ATCONTROLS | FL_PAWN_FAKECLIENT);

	if (pTargetController->IsBot()) {
		cache.m_hGroundEntity.Term();

		if (cache.m_nMoveType == MOVETYPE_NOCLIP) {
			cache.m_nMoveType = MOVETYPE_WALK;
		}
	} else {
		cache.m_hGroundEntity = pTargetPawn->m_hGroundEntity();
	}

	cache.m_sTargetName = pTargetPawn->m_pEntity->m_name.String();
	cache.m_sDesignerName = pTargetPawn->m_pEntity->m_designerName.String();

	if (cache.m_nMoveType == MOVETYPE_NONE) {
		cache.m_nMoveType = MOVETYPE_WALK;
	}

	if (cache.m_nActualMoveType == MOVETYPE_NONE) {
		cache.m_nActualMoveType = MOVETYPE_WALK;
	}

	cache.m_vecLadderNormal = pTargetMoveService->m_vecLadderNormal();
	cache.m_bDucked = pTargetMoveService->m_bDucked();
	cache.m_bDucking = pTargetMoveService->m_bDucking();
	cache.m_fDuckTime = pTargetMoveService->m_flDuckAmount();
	cache.m_fDuckSpeed = pTargetMoveService->m_flDuckSpeed();

	pTargetSurfPlayer->m_pTimerService->BuildSnapshot(cache);
}

void CSurfCheckpointService::LoadCheckpoint(const cp_cache_t& cache) {
	CSurfPlayer* pSurfPlayer = this->GetPlayer();
	if (!pSurfPlayer) {
		return;
	}

	CCSPlayerPawn* pPawn = pSurfPlayer->GetPlayerPawn();
	if (pPawn) {
		return;
	}

	CCSPlayer_MovementServices* pMoveService = pSurfPlayer->GetMoveServices();
	if (!pMoveService) {
		return;
	}

	pSurfPlayer->SetOrigin(cache.m_vecPos);
	pSurfPlayer->SetAngles(cache.m_vecAng);
	pSurfPlayer->SetVelocity(cache.m_vecVel);
	pPawn->m_MoveType(cache.m_nMoveType);
	pPawn->m_nActualMoveType(cache.m_nActualMoveType);
	pPawn->m_flGravityScale(cache.m_fGravity);
	pPawn->m_flVelocityModifier(cache.m_fSpeed);
	pPawn->m_fFlags(cache.m_iFlags);
	pPawn->m_hGroundEntity(cache.m_hGroundEntity);
	pPawn->m_pEntity->m_name = GameEntitySystem()->AllocPooledString(cache.m_sTargetName.c_str());
	pPawn->m_pEntity->m_designerName = GameEntitySystem()->AllocPooledString(cache.m_sDesignerName.c_str());

	pMoveService->m_vecLadderNormal(cache.m_vecLadderNormal);
	pMoveService->m_bDucked(cache.m_bDucked);
	pMoveService->m_bDucking(cache.m_bDucking);
	pMoveService->m_flDuckAmount(cache.m_fDuckTime);
	pMoveService->m_flDuckSpeed(cache.m_fDuckSpeed);

	pSurfPlayer->m_pTimerService->FromSnapshot(cache);
}
