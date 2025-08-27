#include <core/forwards.h>
#include <movement/movement.h>

class CSlopeFix : CMovementForward {
private:
	virtual void OnStartTouchGround(CMovementPlayer* player) override;
};

CSlopeFix g_SlopeFix;

// copy from cs2kz
void CSlopeFix::OnStartTouchGround(CMovementPlayer* player) {
	auto playerPawn = player->GetPlayerPawn();
	CTraceFilterPlayerMovementCS filter(playerPawn);

	Vector ground = player->currentMoveData->m_vecAbsOrigin;
	ground.z -= 2;

	f32 standableZ = 0.7f; // Equal to the mode's cvar.

	bbox_t bounds;
	player->GetBBoxBounds(bounds);
	trace_t trace;
	MEM::CALL::TracePlayerBBox(player->currentMoveData->m_vecAbsOrigin, ground, bounds, &filter, trace);

	// Doesn't hit anything, fall back to the original ground
	if (trace.m_bStartInSolid || trace.m_flFraction == 1.0f) {
		return;
	}

	// TODO: Unhardcode sv_standable_normal
	if (standableZ <= trace.m_vHitNormal.z && trace.m_vHitNormal.z < 1.0f) {
		// Copy the ClipVelocity function from sdk2013
		float backoff;
		float change;
		Vector newVelocity;

		backoff = DotProduct(player->landingVelocity, trace.m_vHitNormal) * 1;

		for (u32 i = 0; i < 3; i++) {
			change = trace.m_vHitNormal[i] * backoff;
			newVelocity[i] = player->landingVelocity[i] - change;
		}

		f32 adjust = DotProduct(newVelocity, trace.m_vHitNormal);
		if (adjust < 0.0f) {
			newVelocity -= (trace.m_vHitNormal * adjust);
		}

		// Make sure the player is going down a ramp by checking if they actually will gain speed from the boost.
		if (newVelocity.Length2D() >= player->landingVelocity.Length2D()) {
			player->currentMoveData->m_vecVelocity.x = newVelocity.x;
			player->currentMoveData->m_vecVelocity.y = newVelocity.y;
			player->landingVelocity.x = newVelocity.x;
			player->landingVelocity.y = newVelocity.y;
		}
	}
}
