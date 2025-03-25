#include "surf_replay.h"
#include <utils/utils.h>

void CSurfReplayPlugin::DoPlayback(CCSPlayerPawn* pBotPawn, CInButtonState& buttons) {
	auto& aFrames = m_umTrackReplays[0];
	auto iFrameSize = aFrames.size();
	if (iFrameSize == 0) {
		return;
	}

	auto slot = pBotPawn->GetController()->GetPlayerSlot();
	auto& currentTick = m_iCurrentTick.at(slot);
	if (currentTick >= iFrameSize) {
		currentTick = 0;
	}

	auto& frame = aFrames.at(currentTick);
	MEM::CALL::SnapViewAngles(pBotPawn, frame.ang);

	auto botFlags = pBotPawn->m_fFlags();
	pBotPawn->m_fFlags(botFlags ^= frame.flags);

	buttons = frame.buttons;
	pBotPawn->SetMoveType(frame.mt);

	Vector& currentPos = pBotPawn->GetAbsOrigin();
	if (currentTick == 0 || currentPos.DistTo(frame.pos) > 15000.0) {
		static Vector zeroVel {0.0f, 0.0f, 0.0f};
		pBotPawn->SetMoveType(MoveType_t::MOVETYPE_NOCLIP);
		pBotPawn->Teleport(&frame.pos, nullptr, &zeroVel);
	} else {
		constexpr auto replay_tickrate = ENGINE_FIXED_TICK_RATE;
		Vector calculatedVelocity = (frame.pos - currentPos) * replay_tickrate;
		pBotPawn->Teleport(nullptr, nullptr, &calculatedVelocity);
	}

	currentTick++;
}
