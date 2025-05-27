#include "cbaseplayerpawn.h"

void CBasePlayerPawn::CommitSuicide(bool bExplode, bool bForce) {
	this->m_bTakesDamage(true);
	static auto iOffset = GAMEDATA::GetOffset("CommitSuicide");
	CALL_VIRTUAL(void, iOffset, this, bExplode, bForce);
	this->m_bTakesDamage(false);
}

void CBasePlayerPawn::SetCollisionGroup(StandardCollisionGroups_t group) {
	this->m_Collision()->m_CollisionGroup(group);
	this->CollisionRulesChanged();
}
