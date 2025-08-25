#include "datatypes.h"
#include <sdk/entity/cbaseplayerpawn.h>

CTraceFilterPlayerMovementCS::CTraceFilterPlayerMovementCS(CBasePlayerPawn* pawn)
	: CTraceFilter(pawn, pawn->m_hOwnerEntity().Get(), pawn->m_Collision()->m_collisionAttribute().m_nHierarchyId(),
				   pawn->m_pCollision()->m_collisionAttribute().m_nInteractsWith(), COLLISION_GROUP_PLAYER, true) {
	EnableInteractsAsLayer(LAYER_INDEX_CONTENTS_PLAYER);
	m_nObjectSetMask = RNQUERY_OBJECTS_ALL;
	m_bHitSolid = true;
	m_bHitSolidRequiresGenerateContacts = true;
	m_bHitTrigger = false;
	m_bShouldIgnoreDisabledPairs = true;
	m_bIgnoreIfBothInteractWithHitboxes = false;
	m_bForceHitEverything = false;
	m_bUnknown = true;
}
