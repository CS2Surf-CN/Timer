#include <surf/replay/surf_replay.h>
#include <surf/timer/surf_timer.h>
#include <utils/utils.h>

void CSurfReplayService::Init() {
	m_bEnabled = false;
	m_iCurrentFrame = 0;
	m_vCurrentFrames = {};
	m_vCurrentFrames.resize(10000);
	m_ExtraStageFrame = {};
	m_ExtraTrackFrame = {};
	m_hTrackPostFrameTimer.Close();
	m_hStagePostFrameTimer.Close();
}

void CSurfReplayService::OnEnterStart_Recording() {
	m_bEnabled = true;

	auto pPlayer = GetPlayer();

	if (m_ExtraStageFrame.bGrabEnd && pPlayer->GetCurrentStage() == 1) {
		FinishGrabbingPostFrames(true);
	}

	if (m_ExtraTrackFrame.bGrabEnd) {
		FinishGrabbingPostFrames();
	}
}

void CSurfReplayService::OnStart_Recording() {
	i64 iMaxPreFrames = std::floor(SURF::ReplayPlugin()->m_cvarTrackPreRunTime->GetFloat() * SURF_TICKRATE);
	i64 iFrameDifference = m_iCurrentFrame - iMaxPreFrames;
	if (iFrameDifference > 0) {
		if (iFrameDifference > 100) {
			// need figure out why diff > 100
			// SDK_ASSERT(false);

			/*for (size_t i = iFrameDifference; i < m_iCurrentFrame; i++) {
				size_t iSwapFrame = i - iFrameDifference;
				std::swap(m_vCurrentFrames.at(i), m_vCurrentFrames.at(iSwapFrame));
			}

			m_iCurrentFrame = iMaxPreFrames;*/
		} else {
			m_vCurrentFrames.erase(m_vCurrentFrames.begin(), m_vCurrentFrames.begin() + iFrameDifference);

			m_iCurrentFrame = 0;
		}
	}

	m_ExtraTrackFrame.iPreEnd = m_iCurrentFrame;
}

void CSurfReplayService::OnTimerFinishPost_SaveRecording() {
	auto pPlayer = GetPlayer();
	if (pPlayer->IsPracticeMode() || m_iCurrentFrame == 0) {
		return;
	}

	m_ExtraTrackFrame.iEndStart = m_iCurrentFrame;

	auto fTrackPostRunTime = SURF::ReplayPlugin()->m_cvarTrackPostRunTime->GetFloat();
	if (fTrackPostRunTime > 0.0f) {
		m_ExtraTrackFrame.bGrabEnd = true;

		m_hTrackPostFrameTimer.Close();
		m_hTrackPostFrameTimer = UTIL::CreateTimer(
			fTrackPostRunTime,
			[](CHandle<CCSPlayerController> hController) {
			auto pController = hController.Get();
			if (!pController || !pController->IsController()) {
				return -1.0;
			}

			auto pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
			if (!pPlayer) {
				SDK_ASSERT(false);
				return -1.0;
			}

			pPlayer->m_pReplayService->FinishGrabbingPostFrames();

			return -1.0;
		},
			pPlayer->GetController()->GetRefEHandle());
	} else {
		FinishGrabbingPostFrames();
	}
}

void CSurfReplayService::FinishGrabbingPostFrames(bool bStage) {
	auto pReplayPlugin = SURF::ReplayPlugin();

	if (bStage) {
		m_ExtraStageFrame.bGrabEnd = false;
		m_hStagePostFrameTimer.Close();
		SaveRecord(true);
	} else {
		m_ExtraTrackFrame.bGrabEnd = false;
		m_hTrackPostFrameTimer.Close();
		SaveRecord(false);
	}
}

void CSurfReplayService::DoRecord(CCSPlayerPawnBase* pawn, const CPlayerButton& buttons, const QAngle& viewAngles) {
	bool bCanRecord = m_bEnabled || m_ExtraStageFrame.bGrabEnd || m_ExtraTrackFrame.bGrabEnd;
	if (!bCanRecord) {
		return;
	}

	if (m_vCurrentFrames.size() <= m_iCurrentFrame) {
		size_t newSize = static_cast<size_t>(m_iCurrentFrame) * SURF_REPLAY_RESIZE_THRESHOLD;
		m_vCurrentFrames.resize(newSize);
	}

	replay_frame_data_t frame;
	frame.ang = viewAngles;
	frame.pos = pawn->GetAbsOrigin();
	frame.buttons = buttons.down | buttons.scroll;
	frame.flags = pawn->m_fFlags();
	frame.mt = pawn->m_MoveType();

	m_vCurrentFrames[m_iCurrentFrame] = frame;
	m_iCurrentFrame++;
}

void CSurfReplayService::SaveRecord(bool bStageReplay, ReplayArray_t* out) {
	auto pPlayer = GetPlayer();
	auto pPlugin = SURF::ReplayPlugin();

	replay_run_info_t info {};
	info.timestamp = std::time(nullptr);
	info.steamid = pPlayer->GetSteamId64();
	info.time = pPlayer->GetCurrentTime();
	info.style = pPlayer->GetStyle();
	info.track = pPlayer->GetCurrentTrack();
	info.stage = bStageReplay ? pPlayer->GetCurrentStage() : 0;
	info.framelength = m_iCurrentFrame - (bStageReplay ? m_ExtraStageFrame.iPreStart : 0);

	ReplayArray_t aFrames = bStageReplay ? UTIL::VECTOR::Slice(m_vCurrentFrames, m_ExtraStageFrame.iPreStart, m_iCurrentFrame) : m_vCurrentFrames;
	aFrames.resize(info.framelength);

	if (out) {
		*out = aFrames;
	}

	pPlugin->AsyncWriteReplayFile(info, aFrames);

	ClearFrames();
}

void CSurfReplayService::ClearFrames() {
	m_bEnabled = false;
	m_vCurrentFrames.clear();
	m_iCurrentFrame = 0;
	m_ExtraStageFrame = {};
	m_ExtraTrackFrame = {};
}
