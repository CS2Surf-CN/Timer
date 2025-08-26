#include "surf_replay.h"
#include <utils/utils.h>

void CSurfBotReplayService::OnInit() {
	Init();
}

void CSurfBotReplayService::OnReset() {
	Init();
}

void CSurfBotReplayService::Init() {
	m_info.iTick = 0;
	m_info.iTrack = -1;
	m_info.iStage = -1;
}

void CSurfBotReplayService::DoPlayback(CCSPlayerPawnBase* pBotPawn, CInButtonState& buttons, QAngle& viewAngles) {
	ReplayArray_t* pFrames = nullptr;
	if (IsTrackBot()) {
		pFrames = &SURF::ReplayPlugin()->m_aTrackReplays.at(m_info.iTrack);
	} else if (IsStageBot()) {
		pFrames = &SURF::ReplayPlugin()->m_aStageReplays.at(m_info.iStage);
	}

	if (!pFrames) {
		return;
	}

	size_t iFrameSize = pFrames->size();
	if (!iFrameSize) {
		return;
	}

	if (m_info.iStatus == Replay_End) {
		return;
	}

	if (m_info.iStatus == Replay_Start) {
	}

	size_t iLimit = iFrameSize;

	if (m_info.iTick >= iLimit) {
		m_info.iTick = iLimit;
		m_info.iRealTick = iLimit;
		m_info.iStatus = Replay_End;
		m_info.hRestartTimer = UTIL::CreateTimer(
			m_info.fRestartDelay,
			[](CHandle<CCSPlayerController> hController) {
			auto pController = hController.Get();
			if (!pController || !pController->IsBot()) {
				return -1.0;
			}

			auto pSurfBot = SURF::GetBotManager()->ToPlayer(pController);
			if (pSurfBot) {
				pSurfBot->m_pReplayService->FinishReplay();
			}

			return -1.0;
		},
			GetPlayer()->GetController()->GetRefEHandle());
		return;
	}

	auto& frame = pFrames->at(m_info.iTick);
	viewAngles = frame.ang;

	auto botFlags = pBotPawn->m_fFlags();
	pBotPawn->m_fFlags(botFlags | frame.flags);

	buttons.down = frame.buttons;

	const Vector& vecCurrentPos = pBotPawn->GetAbsOrigin();
	if (m_info.iTick == 0 || vecCurrentPos.DistTo(frame.pos) > 15000.0) {
		pBotPawn->m_MoveType(MoveType_t::MOVETYPE_NOCLIP);
		pBotPawn->Teleport(&frame.pos, nullptr, &SURF::ZERO_VEC);
	} else {
		pBotPawn->m_MoveType(frame.mt);
		Vector vecCalculatedVelocity = (frame.pos - vecCurrentPos) * SURF_TICKRATE;
		pBotPawn->Teleport(nullptr, nullptr, &vecCalculatedVelocity);
	}

	m_info.iTick += m_info.b2x ? 2 : 1;
	m_info.iRealTick += m_info.b2x ? 2 : 1;
}

void CSurfBotReplayService::StartReplay(const replay_bot_info_t* info) {
	if (info) {
		m_info = *info;
	}
}

void CSurfBotReplayService::FinishReplay(const replay_bot_info_t* info) {
	if (info) {
		m_info = *info;
	}

	switch (m_info.iType) {
		case Replay_Central: {
			break;
		}
		case Replay_Looping: {
			break;
		}
		case Replay_Dynamic: {
			break;
		}
	}
}
