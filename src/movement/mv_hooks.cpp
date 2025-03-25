#include "movement.h"

#include <core/gamedata.h>
#include <core/memory.h>
#include <cs2surf.h>

#include <surf/surf_player.h>
#include <utils/utils.h>

static void* Hook_OnMovementServicesRunCmds(CPlayer_MovementServices* pMovementServices, CUserCmd* pUserCmd) {
	CCSPlayerPawn* pawn = pMovementServices->GetPawn();
	if (!pawn || !pawn->IsAlive()) {
		return MEM::SDKCall<void*>(MOVEMENT::TRAMPOLINE::g_fnMovementServicesRunCmds, pMovementServices, pUserCmd);
	}

	CCSPlayerController* controller = pawn->GetController<CCSPlayerController>();
	if (!controller) {
		return MEM::SDKCall<void*>(MOVEMENT::TRAMPOLINE::g_fnMovementServicesRunCmds, pMovementServices, pUserCmd);
	}

	int32_t seed;
	int32_t weapon;
	int32_t cmdnum;
	int32_t tickcount;
	int32_t mouse[2] = {0, 0};

	float vec[3] = {0.0f, 0.0f, 0.0f};
	QAngle viewAngles = {0.0f, 0.0f, 0.0f};

	CBaseUserCmdPB* baseCmd = pUserCmd->mutable_base();
	if (baseCmd) {
		weapon = baseCmd->weaponselect();
		cmdnum = baseCmd->legacy_command_number();
		tickcount = baseCmd->client_tick();
		seed = baseCmd->random_seed();
		mouse[0] = baseCmd->mousedx();
		mouse[1] = baseCmd->mousedy();
		vec[0] = baseCmd->leftmove();
		vec[1] = baseCmd->forwardmove();
		vec[2] = baseCmd->upmove();
		viewAngles.x = baseCmd->viewangles().x();
		viewAngles.y = baseCmd->viewangles().y();
		viewAngles.z = baseCmd->viewangles().z();
	}

	bool block = false;
	for (auto p = CMovementForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnPlayerRunCmd(pawn, pUserCmd->m_buttons, vec, viewAngles, weapon, cmdnum, tickcount, seed, mouse)) {
			block = true;
		}
	}

	if (baseCmd) {
		// subtick must die
		baseCmd->clear_subtick_moves();

		baseCmd->set_weaponselect(weapon);
		baseCmd->set_legacy_command_number(cmdnum);
		baseCmd->set_client_tick(tickcount);
		baseCmd->set_random_seed(seed);
		baseCmd->set_mousedx(mouse[0]);
		baseCmd->set_mousedy(mouse[1]);
		baseCmd->set_leftmove(vec[0]);
		baseCmd->set_forwardmove(vec[1]);
		baseCmd->set_upmove(vec[2]);

		// unavaliable
		/*if (baseCmd->has_viewangles()) {
			CMsgQAngle* viewangles = baseCmd->mutable_viewangles();
			if (viewangles) {
				viewangles->set_x(viewAngles.x);
				viewangles->set_y(viewAngles.y);
				viewangles->set_z(viewAngles.z);
			}
		}*/
	}

	if (block) {
		return nullptr;
	}

	auto ret = MEM::SDKCall<void*>(MOVEMENT::TRAMPOLINE::g_fnMovementServicesRunCmds, pMovementServices, pUserCmd);

	FORWARD_POST(CMovementForward, OnPlayerRunCmdPost, pawn, pUserCmd->m_buttons, vec, viewAngles, weapon, cmdnum, tickcount, seed, mouse);

	return ret;
}

static void Hook_OnTryPlayerMove(CCSPlayer_MovementServices* ms, CMoveData* mv, Vector* pFirstDest, trace_t* pFirstTrace) {
	bool block = false;
	for (auto p = CMovementForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnTryPlayerMove(ms, mv, pFirstDest, pFirstTrace)) {
			block = true;
		}
	}

	if (block) {
		return;
	}

	MEM::SDKCall<void>(MOVEMENT::TRAMPOLINE::g_fnTryPlayerMove, ms, mv, pFirstDest, pFirstTrace);

	FORWARD_POST(CMovementForward, OnTryPlayerMovePost, ms, mv, pFirstDest, pFirstTrace);
}

static void Hook_OnCategorizePosition(CCSPlayer_MovementServices* ms, CMoveData* mv, bool bStayOnGround) {
	bool block = false;
	for (auto p = CMovementForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnCategorizePosition(ms, mv, bStayOnGround)) {
			block = true;
		}
	}
	if (block) {
		return;
	}

	MEM::SDKCall<void>(MOVEMENT::TRAMPOLINE::g_fnCategorizePosition, ms, mv, bStayOnGround);

	CMovementPlayer* player = MOVEMENT::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return;
	}

	auto playerPawn = player->GetPlayerPawn();
	if (!playerPawn) {
		return;
	}

	bool oldOnGround = !!(playerPawn->m_fFlags() & FL_ONGROUND);
	bool ground = !!(playerPawn->m_fFlags() & FL_ONGROUND);
	if (oldOnGround != ground) {
		if (ground) {
			player->RegisterLanding(mv->m_vecVelocity);
			// player->duckBugged = player->processingDuck;
			FORWARD_POST(CMovementForward, OnStartTouchGround, player);
		} else {
			player->RegisterTakeoff(false);
			FORWARD_POST(CMovementForward, OnStopTouchGround, player);
		}
	}

	FORWARD_POST(CMovementForward, OnCategorizePositionPost, ms, mv, bStayOnGround);
}

static void Hook_OnJump(CCSPlayer_MovementServices* ms, CMoveData* mv) {
	bool block = false;
	for (auto p = CMovementForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnJump(ms, mv)) {
			block = true;
		}
	}
	if (block) {
		return;
	}

	MEM::SDKCall<void>(MOVEMENT::TRAMPOLINE::g_fnJump, ms, mv);

	CMovementPlayer* player = MOVEMENT::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return;
	}

	Vector oldOutWishVel = mv->m_outWishVel;
	MoveType_t oldMoveType = player->GetPlayerPawn()->m_MoveType();
	if (mv->m_outWishVel != oldOutWishVel) {
		player->RegisterTakeoff(true);
		FORWARD_POST(CMovementForward, OnStopTouchGround, player);
	}

	FORWARD_POST(CMovementForward, OnJumpPost, ms, mv);
}

static void Hook_OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) {
	CMovementPlayer* player = MOVEMENT::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return MEM::SDKCall<void>(MOVEMENT::TRAMPOLINE::g_fnProcessMovement, ms, mv);
	}

	player->currentMoveData = mv;
	player->moveDataPre = CMoveData(*mv);
	player->processingMovement = true;

	bool block = false;
	for (auto p = CMovementForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnProcessMovement(ms, mv)) {
			block = true;
		}
	}
	if (block) {
		return;
	}

	MEM::SDKCall<void>(MOVEMENT::TRAMPOLINE::g_fnProcessMovement, ms, mv);

	player->moveDataPost = CMoveData(*mv);
	player->processingMovement = false;

	FORWARD_POST(CMovementForward, OnProcessMovementPost, ms, mv);
}

static void* Hook_OnPhysicsSimulate(CCSPlayerController* pController) {
	if (pController->m_bIsHLTV()) {
		return nullptr;
	}

	if (!pController) {
		return MEM::SDKCall<void*>(MOVEMENT::TRAMPOLINE::g_fnPhysicsSimulate, pController);
	}

	SurfPlugin()->simulatingPhysics = true;

	bool block = false;
	for (auto p = CMovementForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnPhysicsSimulate(pController)) {
			block = true;
		}
	}
	if (block) {
		return nullptr;
	}

	void* ret = MEM::SDKCall<void*>(MOVEMENT::TRAMPOLINE::g_fnPhysicsSimulate, pController);

	SurfPlugin()->simulatingPhysics = false;

	FORWARD_POST(CMovementForward, OnPhysicsSimulatePost, pController);

	return ret;
}

static float Hook_OnGetMaxSpeed(CCSPlayerPawn* pawn) {
	CMovementPlayer* player = MOVEMENT::GetPlayerManager()->ToPlayer(pawn);
	if (!player) {
		return MEM::SDKCall<float>(MOVEMENT::TRAMPOLINE::g_fnGetMaxSpeed, pawn);
	}

	return player->m_fCurrentMaxSpeed;
}

void MOVEMENT::SetupHooks() {
	HOOK_SIG("CPlayer_MovementServices::RunCmds", Hook_OnMovementServicesRunCmds, MOVEMENT::TRAMPOLINE::g_fnMovementServicesRunCmds);
	HOOK_SIG("CCSPlayer_MovementServices::TryPlayerMove", Hook_OnTryPlayerMove, MOVEMENT::TRAMPOLINE::g_fnTryPlayerMove);
	HOOK_SIG("CCSPlayer_MovementServices::CategorizePosition", Hook_OnCategorizePosition, MOVEMENT::TRAMPOLINE::g_fnCategorizePosition);
	HOOK_SIG("CCSPlayer_MovementServices::OnJump", Hook_OnJump, MOVEMENT::TRAMPOLINE::g_fnJump);
	HOOK_SIG("CCSPlayer_MovementServices::ProcessMovement", Hook_OnProcessMovement, MOVEMENT::TRAMPOLINE::g_fnProcessMovement);
	HOOK_SIG("CCSPlayerController::PhysicsSimulate", Hook_OnPhysicsSimulate, MOVEMENT::TRAMPOLINE::g_fnPhysicsSimulate);
	HOOK_SIG("CCSPlayerPawn::GetMaxSpeed", Hook_OnGetMaxSpeed, MOVEMENT::TRAMPOLINE::g_fnGetMaxSpeed);
}

// copy from cs2kz.
void MOVEMENT::ClipVelocity(Vector& in, Vector& normal, Vector& out) {
	f32 backoff = -((in.x * normal.x) + ((normal.z * in.z) + (in.y * normal.y))) * 1;
	backoff = fmaxf(backoff, 0.0) + 0.03125;

	out = normal * backoff + in;
}

bool MOVEMENT::IsValidMovementTrace(trace_t& tr, bbox_t& bounds, CTraceFilterPlayerMovementCS* filter) {
	trace_t stuck;
	// Maybe we don't need this one.
	// if (tr.m_flFraction < FLT_EPSILON)
	//{
	//	return false;
	//}

	if (tr.m_bStartInSolid) {
		return false;
	}

	// We hit something but no valid plane data?
	if (tr.m_flFraction < 1.0f && fabs(tr.m_vHitNormal.x) < FLT_EPSILON && fabs(tr.m_vHitNormal.y) < FLT_EPSILON && fabs(tr.m_vHitNormal.z) < FLT_EPSILON) {
		return false;
	}

	// Is the plane deformed?
	if (fabs(tr.m_vHitNormal.x) > 1.0f || fabs(tr.m_vHitNormal.y) > 1.0f || fabs(tr.m_vHitNormal.z) > 1.0f) {
		return false;
	}

	// Do an unswept trace and a backward trace just to be sure.
	MEM::CALL::TracePlayerBBox(tr.m_vEndPos, tr.m_vEndPos, bounds, filter, stuck);
	if (stuck.m_bStartInSolid || stuck.m_flFraction < 1.0f - FLT_EPSILON) {
		return false;
	}

	MEM::CALL::TracePlayerBBox(tr.m_vEndPos, tr.m_vStartPos, bounds, filter, stuck);
	// For whatever reason if you can hit something in only one direction and not the other way around.
	// Only happens since Call to Arms update, so this fraction check is commented out until it is fixed.
	if (stuck.m_bStartInSolid /*|| stuck.m_flFraction < 1.0f - FLT_EPSILON*/) {
		return false;
	}

	return true;
}
