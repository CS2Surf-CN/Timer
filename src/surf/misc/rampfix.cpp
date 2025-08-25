#include <surf/surf_player.h>
#include <pch.h>
#include <utils/utils.h>
#include <surf/misc/surf_misc.h>

// copy from cs2kz

constexpr auto MAX_BUMPS = 4;
constexpr auto RAMP_PIERCE_DISTANCE = 0.75f;
constexpr auto RAMP_BUG_THRESHOLD = 0.99f;
constexpr auto RAMP_BUG_VELOCITY_THRESHOLD = 0.95f;
constexpr auto NEW_RAMP_THRESHOLD = 0.95f;

class CRampfix : CMovementForward {
private:
	virtual bool OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) override;
	virtual void OnProcessMovementPost(CCSPlayer_MovementServices* ms, const CMoveData* mv) override;
	virtual bool OnTryPlayerMove(CCSPlayer_MovementServices* ms, CMoveData* mv, Vector* pFirstDest, trace_t* pFirstTrace, bool* bIsSurfing) override;
	virtual void OnTryPlayerMovePost(CCSPlayer_MovementServices* ms, const CMoveData* mv, const Vector* pFirstDest, const trace_t* pFirstTrace, const bool* bIsSurfing) override;
	virtual bool OnCategorizePosition(CCSPlayer_MovementServices* ms, CMoveData* mv, bool bStayOnGround) override;
};

CRampfix g_Rampfix;

bool CRampfix::OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return true;
	}

	player->m_pMiscService->didTPM = false;

	return true;
}

void CRampfix::OnProcessMovementPost(CCSPlayer_MovementServices* ms, const CMoveData* mv) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return;
	}

	auto& pMiscService = player->m_pMiscService;
	if (!pMiscService->didTPM) {
		pMiscService->lastValidPlane = vec3_origin;
	}
}

bool CRampfix::OnTryPlayerMove(CCSPlayer_MovementServices* ms, CMoveData* mv, Vector* pFirstDest, trace_t* pFirstTrace, bool* bIsSurfing) {
	CCSPlayerPawnBase* pawn = ms->GetPawn();
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pawn);
	if (!player) {
		return true;
	}

	auto& pMiscService = player->m_pMiscService;
	pMiscService->overrideTPM = false;
	pMiscService->didTPM = true;

	f32 timeLeft = UTIL::GetGlobals()->frametime;

	Vector start, velocity, end;
	player->GetOrigin(start);
	player->GetVelocity(velocity);

	if (velocity.Length() == 0.0f) {
		// No move required.
		return true;
	}

	Vector primalVelocity = velocity;
	bool validPlane {};

	f32 allFraction {};
	trace_t pm;
	u32 bumpCount {};
	Vector planes[5];
	u32 numPlanes {};
	trace_t pierce;

	bbox_t bounds;
	bounds.mins = {-16.0f, -16.0f, 0.0f};
	bounds.maxs = {16.0f, 16.0f, 72.0f};
	if (ms->m_bDucked()) {
		bounds.maxs.z = 54.0f;
	}

	CTraceFilterPlayerMovementCS filter(pawn);

	bool potentiallyStuck {};

	for (bumpCount = 0; bumpCount < MAX_BUMPS; bumpCount++) {
		// Assume we can move all the way from the current origin to the end point.
		VectorMA(start, timeLeft, velocity, end);
		// See if we can make it from origin to end point.
		// If their velocity Z is 0, then we can avoid an extra trace here during WalkMove.
		if (pFirstDest && end == *pFirstDest) {
			pm = *pFirstTrace;
		} else {
			MEM::CALL::TracePlayerBBox(start, end, bounds, &filter, pm);
			if (end == start) {
				continue;
			}

			if (MOVEMENT::IsValidMovementTrace(pm, bounds, &filter) && pm.m_flFraction == 1.0f) {
				// Player won't hit anything, nothing to do.
				break;
			}

			if (pMiscService->lastValidPlane.Length() > FLT_EPSILON && (!MOVEMENT::IsValidMovementTrace(pm, bounds, &filter) || pm.m_vHitNormal.Dot(pMiscService->lastValidPlane) < RAMP_BUG_THRESHOLD || (potentiallyStuck && pm.m_flFraction == 0.0f))) {
				// We hit a plane that will significantly change our velocity. Make sure that this plane is significant
				// enough.
				Vector offsetDirection;
				f32 offsets[] = {0.0f, -1.0f, 1.0f};
				bool success {};
				for (u32 i = 0; i < 3 && !success; i++) {
					for (u32 j = 0; j < 3 && !success; j++) {
						for (u32 k = 0; k < 3 && !success; k++) {
							if (i == 0 && j == 0 && k == 0) {
								offsetDirection = pMiscService->lastValidPlane;
							} else {
								offsetDirection = {offsets[i], offsets[j], offsets[k]};
								// Check if this random offset is even valid.
								if (pMiscService->lastValidPlane.Dot(offsetDirection) <= 0.0f) {
									continue;
								}
								trace_t test;
								MEM::CALL::TracePlayerBBox(start + offsetDirection * RAMP_PIERCE_DISTANCE, start, bounds, &filter, test);
								if (!MOVEMENT::IsValidMovementTrace(test, bounds, &filter)) {
									continue;
								}
							}
							bool goodTrace {};
							f32 ratio {};
							bool hitNewPlane {};
							for (ratio = 0.25f; ratio <= 1.0f; ratio += 0.25f) {
								MEM::CALL::TracePlayerBBox(start + offsetDirection * RAMP_PIERCE_DISTANCE * ratio, end + offsetDirection * RAMP_PIERCE_DISTANCE * ratio, bounds, &filter, pierce);
								if (!MOVEMENT::IsValidMovementTrace(pierce, bounds, &filter)) {
									continue;
								}
								// Try until we hit a similar plane.
								// clang-format off
								validPlane = pierce.m_flFraction < 1.0f && pierce.m_flFraction > 0.1f 
											 && pierce.m_vHitNormal.Dot(pMiscService->lastValidPlane) >= RAMP_BUG_THRESHOLD;

								hitNewPlane = pm.m_vHitNormal.Dot(pierce.m_vHitNormal) < NEW_RAMP_THRESHOLD 
											  && pMiscService->lastValidPlane.Dot(pierce.m_vHitNormal) > NEW_RAMP_THRESHOLD;
								// clang-format on
								goodTrace = CloseEnough(pierce.m_flFraction, 1.0f, FLT_EPSILON) || validPlane;
								if (goodTrace) {
									break;
								}
							}
							if (goodTrace || hitNewPlane) {
								// Trace back to the original end point to find its normal.
								trace_t test;
								MEM::CALL::TracePlayerBBox(pierce.m_vEndPos, end, bounds, &filter, test);
								pm = pierce;
								pm.m_vStartPos = start;
								pm.m_flFraction = Clamp((pierce.m_vEndPos - pierce.m_vStartPos).Length() / (end - start).Length(), 0.0f, 1.0f);
								pm.m_vEndPos = test.m_vEndPos;
								if (pierce.m_vHitNormal.Length() > 0.0f) {
									pm.m_vHitNormal = pierce.m_vHitNormal;
									pMiscService->lastValidPlane = pierce.m_vHitNormal;
								} else {
									pm.m_vHitNormal = test.m_vHitNormal;
									pMiscService->lastValidPlane = test.m_vHitNormal;
								}
								success = true;
								pMiscService->overrideTPM = true;
							}
						}
					}
				}
			}
			if (pm.m_vHitNormal.Length() > 0.99f) {
				pMiscService->lastValidPlane = pm.m_vHitNormal;
			}
			potentiallyStuck = pm.m_flFraction == 0.0f;
		}

		if (pm.m_flFraction * velocity.Length() > 0.03125f || pm.m_flFraction > 0.03125f) {
			allFraction += pm.m_flFraction;
			start = pm.m_vEndPos;
			numPlanes = 0;
		}

		if (allFraction == 1.0f) {
			break;
		}

		timeLeft -= UTIL::GetGlobals()->frametime * pm.m_flFraction;

		// 2024-11-07 update also adds a low velocity check... This is only correct as long as you don't collide with other players.
		if (numPlanes >= 5 || (pm.m_vHitNormal.z >= 0.7f && velocity.Length2D() < 1.0f)) {
			VectorCopy(vec3_origin, velocity);
			break;
		}

		planes[numPlanes] = pm.m_vHitNormal;
		numPlanes++;

		if (numPlanes == 1 && pawn->m_MoveType() == MOVETYPE_WALK && pawn->m_hGroundEntity().Get() == nullptr) {
			MOVEMENT::ClipVelocity(velocity, planes[0], velocity);
		} else {
			u32 i, j;
			for (i = 0; i < numPlanes; i++) {
				MOVEMENT::ClipVelocity(velocity, planes[i], velocity);
				for (j = 0; j < numPlanes; j++) {
					if (j != i) {
						// Are we now moving against this plane?
						if (velocity.Dot(planes[j]) < 0) {
							break; // not ok
						}
					}
				}

				if (j == numPlanes) // Didn't have to clip, so we're ok
				{
					break;
				}
			}
			// Did we go all the way through plane set
			if (i != numPlanes) { // go along this plane
				// pmove.velocity is set in clipping call, no need to set again.
				;
			} else { // go along the crease
				if (numPlanes != 2) {
					VectorCopy(vec3_origin, velocity);
					break;
				}
				Vector dir;
				f32 d;
				CrossProduct(planes[0], planes[1], dir);
				// Yes, that's right, you need to do this twice because running it once won't ensure that this will be fully normalized.
				dir.NormalizeInPlace();
				dir.NormalizeInPlace();
				d = dir.Dot(velocity);
				VectorScale(dir, d, velocity);

				if (velocity.Dot(primalVelocity) <= 0) {
					velocity = vec3_origin;
					break;
				}
			}
		}
	}
	pMiscService->tpmOrigin = pm.m_vEndPos;
	pMiscService->tpmVelocity = velocity;

	return true;
}

void CRampfix::OnTryPlayerMovePost(CCSPlayer_MovementServices* ms, const CMoveData* mv, const Vector* pFirstDest, const trace_t* pFirstTrace, const bool* bIsSurfing) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return;
	}

	Vector velocity;
	player->GetVelocity(velocity);
	auto& pMiscService = player->m_pMiscService;
	bool velocityHeavilyModified = pMiscService->tpmVelocity.Normalized().Dot(velocity.Normalized()) < RAMP_BUG_THRESHOLD || (pMiscService->tpmVelocity.Length() > 50.0f && velocity.Length() / pMiscService->tpmVelocity.Length() < RAMP_BUG_VELOCITY_THRESHOLD);
	if (pMiscService->overrideTPM && velocityHeavilyModified && pMiscService->tpmOrigin != vec3_invalid && pMiscService->tpmVelocity != vec3_invalid) {
		player->SetOrigin(pMiscService->tpmOrigin);
		player->SetVelocity(pMiscService->tpmVelocity);
	}
}

bool CRampfix::OnCategorizePosition(CCSPlayer_MovementServices* ms, CMoveData* mv, bool bStayOnGround) {
	CCSPlayerPawnBase* pawn = ms->GetPawn();
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return true;
	}

	auto& pMiscService = player->m_pMiscService;

	// Already on the ground?
	// If we are already colliding on a standable valid plane, we don't want to do the check.
	if (bStayOnGround || pMiscService->lastValidPlane.Length() < EPSILON || pMiscService->lastValidPlane.z > 0.7f) {
		return true;
	}

	// Only attempt to fix rampbugs while going down significantly enough.
	if (player->currentMoveData->m_vecVelocity.z > -64.0f) {
		return true;
	}

	bbox_t bounds;
	bounds.mins = {-16.0f, -16.0f, 0.0f};
	bounds.maxs = {16.0f, 16.0f, 72.0f};
	if (ms->m_bDucked()) {
		bounds.maxs.z = 54.0f;
	}

	CTraceFilterPlayerMovementCS filter(pawn);

	trace_t trace;
	Vector origin, groundOrigin;
	player->GetOrigin(origin);
	groundOrigin = origin;
	groundOrigin.z -= 2.0f;

	MEM::CALL::TracePlayerBBox(origin, groundOrigin, bounds, &filter, trace);

	if (trace.m_flFraction == 1.0f) {
		return true;
	}

	// Is this something that you should be able to actually stand on?
	if (trace.m_flFraction < 0.95f && trace.m_vHitNormal.z > 0.7f && pMiscService->lastValidPlane.Dot(trace.m_vHitNormal) < RAMP_BUG_THRESHOLD) {
		origin += pMiscService->lastValidPlane * 0.0625f;
		groundOrigin = origin;
		groundOrigin.z -= 2.0f;
		MEM::CALL::TracePlayerBBox(origin, groundOrigin, bounds, &filter, trace);
		if (trace.m_bStartInSolid) {
			return true;
		}

		if (trace.m_flFraction == 1.0f || pMiscService->lastValidPlane.Dot(trace.m_vHitNormal) >= RAMP_BUG_THRESHOLD) {
			player->SetOrigin(origin);
		}
	}

	return true;
}
