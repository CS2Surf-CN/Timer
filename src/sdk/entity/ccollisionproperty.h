#pragma once

#include <sdk/schema.h>

class CBaseModelEntity;

enum SurroundingBoundsType_t : uint8_t {
	USE_OBB_COLLISION_BOUNDS = 0x0,
	USE_BEST_COLLISION_BOUNDS = 0x1,
	USE_HITBOXES = 0x2,
	USE_SPECIFIED_BOUNDS = 0x3,
	USE_GAME_CODE = 0x4,
	USE_ROTATION_EXPANDED_BOUNDS = 0x5,
	USE_ROTATION_EXPANDED_ORIENTED_BOUNDS = 0x6,
	USE_COLLISION_BOUNDS_NEVER_VPHYSICS = 0x7,
	USE_ROTATION_EXPANDED_SEQUENCE_BOUNDS = 0x8,
	SURROUNDING_TYPE_BIT_COUNT = 0x3,
};

struct VPhysicsCollisionAttribute_t {
	DECLARE_SCHEMA_CLASS_INLINE(VPhysicsCollisionAttribute_t);

	SCHEMA_FIELD(uint8, m_nCollisionGroup);
	SCHEMA_FIELD(uint64, m_nInteractsAs);
	SCHEMA_FIELD(uint64, m_nInteractsWith);
	SCHEMA_FIELD(uint16, m_nHierarchyId);
};

class CCollisionProperty {
public:
	DECLARE_SCHEMA_STRUCT(CCollisionProperty);

	SCHEMA_FIELD(VPhysicsCollisionAttribute_t, m_collisionAttribute);
	SCHEMA_FIELD(Vector, m_vecMins);
	SCHEMA_FIELD(Vector, m_vecMaxs);
	SCHEMA_FIELD(SolidType_t, m_nSolidType);
	SCHEMA_FIELD(uint8, m_usSolidFlags);
	SCHEMA_FIELD(uint8, m_CollisionGroup);
	SCHEMA_FIELD(Vector, m_vecSpecifiedSurroundingMins);
	SCHEMA_FIELD(Vector, m_vecSpecifiedSurroundingMaxs);
	SCHEMA_FIELD(Vector, m_vecSurroundingMaxs);
	SCHEMA_FIELD(Vector, m_vecSurroundingMins);
	SCHEMA_FIELD(SurroundingBoundsType_t, m_nSurroundType);
	SCHEMA_FIELD(uint8, m_nEnablePhysics);
	SCHEMA_FIELD(float, m_flBoundingRadius);

	CBaseModelEntity* GetOuter() {
		return *(CBaseModelEntity**)((char*)this + 8);
	}
};
