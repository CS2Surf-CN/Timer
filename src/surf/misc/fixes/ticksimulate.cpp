#include <pch.h>
#include <movement/movement.h>
#include <utils/utils.h>

constexpr auto TARGET_TICK = 100.0f;

constexpr auto SUBTICK_INTERPOLATE = ENGINE_FIXED_TICK_RATE / TARGET_TICK;

#define KZ_INTERPOLATE_VIEWANGLE 0

class CTickSimulate : CCoreForward, CMovementForward {
private:
	virtual bool OnPhysicsSimulate(CCSPlayerController* pController) override;

	virtual void OnPhysicsSimulatePost(CCSPlayerController* pController) override;

	virtual bool OnSetupMove(CCSPlayer_MovementServices* ms, CUserCmd* cmd, CMoveData* mv) override;

#if KZ_INTERPOLATE_VIEWANGLE
	virtual bool OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) override;

	virtual void OnProcessMovementPost(CCSPlayer_MovementServices* ms, const CMoveData* mv) override;
#endif
};

CTickSimulate g_TickSimulate;

bool CTickSimulate::OnPhysicsSimulate(CCSPlayerController* pController) {
	auto pMovementService = pController->GetPlayerPawn()->m_pMovementServices();
	u32 tickCount = UTIL::GetGlobals()->tickcount;

	f32 subtickMoveTime = (tickCount - SUBTICK_INTERPOLATE) * ENGINE_FIXED_TICK_INTERVAL;
	for (u32 i = 0; i < 4; i++) {
		f32 when = pMovementService->m_arrForceSubtickMoveWhen()[i];
		if (fabs(subtickMoveTime - when) < 0.001) {
			return true;
		}

		if (subtickMoveTime > when) {
			pMovementService->m_arrForceSubtickMoveWhen(subtickMoveTime, i);
			return true;
		}
	}

	return true;
}

void CTickSimulate::OnPhysicsSimulatePost(CCSPlayerController* pController) {
	auto pMovementService = pController->GetPlayerPawn()->m_pMovementServices();
	u32 tickCount = UTIL::GetGlobals()->tickcount;

	f32 subtickMoveTime = (tickCount + SUBTICK_INTERPOLATE) * ENGINE_FIXED_TICK_INTERVAL;
	for (u32 i = 0; i < 4; i++) {
		f32 when = pMovementService->m_arrForceSubtickMoveWhen()[i];
		if (fabs(subtickMoveTime - when) < 0.001) {
			subtickMoveTime += ENGINE_FIXED_TICK_INTERVAL;
			continue;
		}

		if (subtickMoveTime > when) {
			pMovementService->m_arrForceSubtickMoveWhen(subtickMoveTime, i);
			subtickMoveTime += ENGINE_FIXED_TICK_INTERVAL;
		}
	}
}

bool CTickSimulate::OnSetupMove(CCSPlayer_MovementServices* ms, CUserCmd* cmd, CMoveData* mv) {
	auto pUserCmdPB = cmd->mutable_base();
	for (i32 i = 0; i < pUserCmdPB->subtick_moves_size(); i++) {
		auto pSubtickMove = pUserCmdPB->mutable_subtick_moves(i);
		auto nSubtickButton = pSubtickMove->button();
		if (nSubtickButton == IN_ATTACK || nSubtickButton == IN_ATTACK2 || nSubtickButton == IN_RELOAD) {
			continue;
		}

		float flWhen = pSubtickMove->when();
		//if (nSubtickButton == IN_JUMP) {
		//	i32 iTickCount = UTIL::GetGlobals()->tickcount;
		//	f32 flInputTime = (iTickCount + flWhen - 1) * ENGINE_FIXED_TICK_INTERVAL;
		//	if (flWhen != 0.f) {
		//	
		//	}
		//}
		pSubtickMove->set_when(flWhen >= SUBTICK_INTERPOLATE ? SUBTICK_INTERPOLATE : 0);
	}

	return true;
}

#if KZ_INTERPOLATE_VIEWANGLE
bool CTickSimulate::OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) {
	f64 subtickFraction, whole;
	subtickFraction = modf((f64)UTIL::GetGlobals()->curtime * ENGINE_FIXED_TICK_RATE, &whole);
	if (subtickFraction < 0.001) {
		return true;
	}

	auto pMovementPlayer = MOVEMENT::GetPlayerManager()->ToPlayer(ms);
	if (!pMovementPlayer) {
		return true;
	}

	// First half of the movement, tweak the angle to be the middle of the desired angle and the last angle
	QAngle newAngles = mv->m_vecViewAngles;
	QAngle oldAngles = pMovementPlayer->hasValidDesiredViewAngle ? pMovementPlayer->lastValidDesiredViewAngle : pMovementPlayer->moveDataPost.m_vecViewAngles;
	if (newAngles[YAW] - oldAngles[YAW] > 180) {
		newAngles[YAW] -= 360.0f;
	} else if (newAngles[YAW] - oldAngles[YAW] < -180) {
		newAngles[YAW] += 360.0f;
	}

	for (u32 i = 0; i < 3; i++) {
		newAngles[i] += oldAngles[i];
		newAngles[i] *= SUBTICK_INTERPOLATE;
	}

	mv->m_vecViewAngles = newAngles;

	return true;
}

void CTickSimulate::OnProcessMovementPost(CCSPlayer_MovementServices* ms, const CMoveData* mv) {
	auto pMovementPlayer = MOVEMENT::GetPlayerManager()->ToPlayer(ms);
	if (!pMovementPlayer) {
		return;
	}

	pMovementPlayer->currentMoveData->m_vecViewAngles = pMovementPlayer->moveDataPre.m_vecViewAngles;
	if (UTIL::GetGlobals()->frametime > 0.0f) {
		pMovementPlayer->hasValidDesiredViewAngle = true;
		pMovementPlayer->lastValidDesiredViewAngle = pMovementPlayer->currentMoveData->m_vecViewAngles;
	}
}
#endif
