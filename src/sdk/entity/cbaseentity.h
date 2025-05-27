#pragma once

#include <ehandle.h>
#include <core/gamedata.h>
#include <sdk/schema.h>
#include <sdk/entity/ccollisionproperty.h>

enum S2EdictState {
	FL_EDICT_FULL_S2 = 0x1,
	FL_FULL_EDICT_CHANGED_S2 = 0x2,
	FL_EDICT_ALWAYS_S2 = 0x4,
	FL_EDICT_DONTSEND_S2 = 0x8,
	FL_EDICT_PVSCHECK_S2 = 0x10,
};

enum class PlayerAnimEvent_t : uint32_t {
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY = 0x0,
	PLAYERANIMEVENT_FIRE_GUN_SECONDARY = 0x1,
	PLAYERANIMEVENT_GRENADE_PULL_PIN = 0x2,
	PLAYERANIMEVENT_THROW_GRENADE = 0x3,
	PLAYERANIMEVENT_JUMP = 0x4,
	PLAYERANIMEVENT_RELOAD = 0x5,
	PLAYERANIMEVENT_CLEAR_FIRING = 0x6,
	PLAYERANIMEVENT_DEPLOY = 0x7,
	PLAYERANIMEVENT_SILENCER_STATE = 0x8,
	PLAYERANIMEVENT_SILENCER_TOGGLE = 0x9,
	PLAYERANIMEVENT_THROW_GRENADE_UNDERHAND = 0xa,
	PLAYERANIMEVENT_CATCH_WEAPON = 0xb,
	PLAYERANIMEVENT_LOOKATWEAPON_REQUEST = 0xc,
	PLAYERANIMEVENT_RELOAD_CANCEL_LOOKATWEAPON = 0xd,
	PLAYERANIMEVENT_HAULBACK = 0xe,
	PLAYERANIMEVENT_IDLE = 0xf,
	PLAYERANIMEVENT_STRIKE_HIT = 0x10,
	PLAYERANIMEVENT_STRIKE_MISS = 0x11,
	PLAYERANIMEVENT_BACKSTAB = 0x12,
	PLAYERANIMEVENT_DRYFIRE = 0x13,
	PLAYERANIMEVENT_FIDGET = 0x14,
	PLAYERANIMEVENT_RELEASE = 0x15,
	PLAYERANIMEVENT_TAUNT = 0x16,
	PLAYERANIMEVENT_COUNT = 0x17,
};

enum PointWorldTextJustifyHorizontal_t : uint32_t {
	POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_LEFT = 0x0,
	POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_CENTER = 0x1,
	POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_RIGHT = 0x2,
};

enum PointWorldTextJustifyVertical_t : uint32_t {
	POINT_WORLD_TEXT_JUSTIFY_VERTICAL_BOTTOM = 0x0,
	POINT_WORLD_TEXT_JUSTIFY_VERTICAL_CENTER = 0x1,
	POINT_WORLD_TEXT_JUSTIFY_VERTICAL_TOP = 0x2,
};

enum PointWorldTextReorientMode_t : uint32_t {
	POINT_WORLD_TEXT_REORIENT_NONE = 0x0,
	POINT_WORLD_TEXT_REORIENT_AROUND_UP = 0x1,
};

enum HierarchyType_t : uint8_t {
	HIERARCHY_NONE = 0x0,
	HIERARCHY_BONE_MERGE = 0x1,
	HIERARCHY_ATTACHMENT = 0x2,
	HIERARCHY_ABSORIGIN = 0x3,
	HIERARCHY_BONE = 0x4,
	HIERARCHY_TYPE_COUNT = 0x5,
};

class CCollisionProperty;

class CNetworkedQuantizedFloat {
public:
	float32 m_Value;
	uint16 m_nEncoder;
	bool m_bUnflattened;
};

class CNetworkOriginCellCoordQuantizedVector {
public:
	DECLARE_SCHEMA_CLASS_INLINE(CNetworkOriginCellCoordQuantizedVector);

	SCHEMA_FIELD(uint16, m_cellX);
	SCHEMA_FIELD(uint16, m_cellY);
	SCHEMA_FIELD(uint16, m_cellZ);
	SCHEMA_FIELD(uint16, m_nOutsideWorld);

	SCHEMA_FIELD(float, m_vecX);
	SCHEMA_FIELD(float, m_vecY);
	SCHEMA_FIELD(float, m_vecZ);
};

class CNetworkVelocityVector {
public:
	DECLARE_SCHEMA_CLASS_INLINE(CNetworkVelocityVector);

	SCHEMA_FIELD(float, m_vecX);
	SCHEMA_FIELD(float, m_vecY);
	SCHEMA_FIELD(float, m_vecZ);
};

class CNetworkViewOffsetVector {
public:
	DECLARE_SCHEMA_STRUCT(CNetworkViewOffsetVector);

	SCHEMA_FIELD(float, m_vecX);
	SCHEMA_FIELD(float, m_vecY);
	SCHEMA_FIELD(float, m_vecZ);
};

class CGameSceneNode {
public:
	DECLARE_SCHEMA_CLASS(CGameSceneNode);

	SCHEMA_FIELD_SKELETON(CTransform, m_nodeToWorld);
	SCHEMA_FIELD_SKELETON(CEntityInstance*, m_pOwner);
	SCHEMA_FIELD_SKELETON(CGameSceneNode*, m_pParent);
	SCHEMA_FIELD_SKELETON(CGameSceneNode*, m_pChild);
	SCHEMA_FIELD_SKELETON(CGameSceneNode*, m_pNextSibling);
	SCHEMA_FIELD_SKELETON(Vector, m_vecOrigin);
	SCHEMA_FIELD_SKELETON(QAngle, m_angRotation);
	SCHEMA_FIELD_SKELETON(QAngle, m_angAbsRotation);
	SCHEMA_FIELD_SKELETON(float, m_flScale);
	SCHEMA_FIELD_SKELETON(float, m_flAbsScale);
	SCHEMA_FIELD_SKELETON(Vector, m_vecAbsOrigin);
	SCHEMA_FIELD_SKELETON(Vector, m_vRenderOrigin);
	SCHEMA_FIELD_SKELETON(bool, m_bForceParentToBeNetworked);
	SCHEMA_FIELD_SKELETON(uint8_t, m_nHierarchicalDepth);
	SCHEMA_FIELD_SKELETON(HierarchyType_t, m_nHierarchyType);
};

class CModelState {
public:
	DECLARE_SCHEMA_STRUCT(CModelState);

	SCHEMA_FIELD(CUtlSymbolLarge, m_ModelName);
	SCHEMA_FIELD(uint64_t, m_MeshGroupMask);
};

class CSkeletonInstance : public CGameSceneNode {
public:
	DECLARE_SCHEMA_CLASS(CSkeletonInstance);

	SCHEMA_FIELD_SKELETON(CModelState, m_modelState);
};

class CBodyComponent {
public:
	DECLARE_SCHEMA_CLASS(CBodyComponent);

	SCHEMA_FIELD(CGameSceneNode*, m_pSceneNode);
};

class CNetworkTransmitComponent {
public:
	DECLARE_SCHEMA_CLASS(CNetworkTransmitComponent);

	SCHEMA_FIELD(uint8_t, m_nTransmitStateOwnedCounter);

	// str -> "FL_FULL_EDICT_CHANGED", ^ if ( (m_fStateFlags & 2) != 0 )
	SCHEMA_FIELD_CUSTOM(int, m_iStateFlags, 0x180);
};

class CBaseEntity : public CEntityInstance {
public:
	DECLARE_SCHEMA_CLASS(CBaseEntity);

	SCHEMA_FIELD(CBodyComponent*, m_CBodyComponent);
	SCHEMA_FIELD(CBitVec<64>, m_isSteadyState);
	SCHEMA_FIELD(float, m_lastNetworkChange);
	SCHEMA_FIELD_POINTER(CNetworkTransmitComponent, m_NetworkTransmitComponent);
	SCHEMA_FIELD(int, m_iHealth);
	SCHEMA_FIELD(uint8, m_lifeState);
	SCHEMA_FIELD(int, m_iTeamNum);
	SCHEMA_FIELD(CUtlString, m_sUniqueHammerID);
	SCHEMA_FIELD(bool, m_bTakesDamage);
	SCHEMA_FIELD(MoveType_t, m_MoveType);
	SCHEMA_FIELD(MoveType_t, m_nActualMoveType);
	SCHEMA_FIELD(Vector, m_vecBaseVelocity);
	SCHEMA_FIELD(Vector, m_vecAbsVelocity);
	SCHEMA_FIELD(CNetworkVelocityVector, m_vecVelocity);
	SCHEMA_FIELD(CCollisionProperty*, m_pCollision);
	SCHEMA_FIELD(CHandle<CBaseEntity>, m_hGroundEntity);
	SCHEMA_FIELD(uint32_t, m_fFlags);
	SCHEMA_FIELD(int32_t, m_iEFlags);
	SCHEMA_FIELD(float, m_flGravityScale);
	SCHEMA_FIELD(float, m_flWaterLevel);
	SCHEMA_FIELD(int, m_fEffects);
	SCHEMA_FIELD(CHandle<CBaseEntity>, m_hOwnerEntity);
	SCHEMA_FIELD(QAngle, m_vecAngVelocity);

	int entindex() {
		return m_pEntity->m_EHandle.GetEntryIndex();
	}

	bool IsPawn() {
		static auto iOffset = GAMEDATA::GetOffset("IsEntityPawn");
		return CALL_VIRTUAL(bool, iOffset, this);
	}

	bool IsController() {
		static auto iOffset = GAMEDATA::GetOffset("IsEntityController");
		return CALL_VIRTUAL(bool, iOffset, this);
	}

	bool IsAlive() {
		return this->m_lifeState() == LIFE_ALIVE;
	}

	void SetMoveType(MoveType_t movetype) {
		this->m_MoveType(movetype);
		this->m_nActualMoveType(movetype);
	}

	void CollisionRulesChanged() {
		static auto iOffset = GAMEDATA::GetOffset("CollisionRulesChanged");
		CALL_VIRTUAL(void, iOffset, this);
	}

	int GetTeam() {
		return m_iTeamNum();
	}

	void StartTouch(CBaseEntity* pOther) {
		static auto iOffset = GAMEDATA::GetOffset("CBaseEntity::StartTouch");
		CALL_VIRTUAL(bool, iOffset, this, pOther);
	}

	void Touch(CBaseEntity* pOther) {
		static auto iOffset = GAMEDATA::GetOffset("CBaseEntity::Touch");
		CALL_VIRTUAL(bool, iOffset, this, pOther);
	}

	void EndTouch(CBaseEntity* pOther) {
		static auto iOffset = GAMEDATA::GetOffset("CBaseEntity::EndTouch");
		CALL_VIRTUAL(bool, iOffset, this, pOther);
	}

	void Teleport(const Vector* newPosition, const QAngle* newAngles, const Vector* newVelocity) {
		static auto iOffset = GAMEDATA::GetOffset("CBaseEntity::Teleport");
		CALL_VIRTUAL(bool, iOffset, this, newPosition, newAngles, newVelocity);
	}

	void Kill() {
		this->AcceptInput("kill");
	}

	void AcceptInput(const char* pInputName, variant_t value = variant_t(""), CEntityInstance* pActivator = nullptr, CEntityInstance* pCaller = nullptr);

	void DispatchSpawn(CEntityKeyValues* pInitKeyValue = nullptr);
	void SetParent(CBaseEntity* pParent);
	void SetName(const char* pszName, bool bCheckDuplicate = false);

	const Vector& GetAbsOrigin();
	const Vector& GetOrigin();
	const QAngle& GetAbsAngles();
};

struct EntityInputData_t {
	CBaseEntity* pActivator;
	CBaseEntity* pCaller;
	variant_t value;
	int nOutputID;
};

class CBaseModelEntity : public CBaseEntity {
public:
	DECLARE_SCHEMA_CLASS(CBaseModelEntity);

	SCHEMA_FIELD_POINTER(CCollisionProperty, m_Collision);
	SCHEMA_FIELD(Color, m_clrRender);
	SCHEMA_FIELD(RenderMode_t, m_nRenderMode);
	SCHEMA_FIELD(float, m_fadeMinDist);
	SCHEMA_FIELD(CNetworkViewOffsetVector, m_vecViewOffset);
};

class CBaseViewModel : public CBaseModelEntity {
public:
	DECLARE_SCHEMA_CLASS(CBaseViewModel);

	SCHEMA_FIELD(int, m_nViewModelIndex);
};

class CBaseAnimGraph : public CBaseModelEntity {
public:
	DECLARE_SCHEMA_CLASS(CBaseAnimGraph);
};

class CBaseFlex : public CBaseAnimGraph {
public:
	DECLARE_SCHEMA_CLASS(CBaseFlex);
};

class CEconEntity : public CBaseFlex {
public:
	DECLARE_SCHEMA_CLASS(CEconEntity);
};

class CBasePlayerWeapon : public CEconEntity {
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerWeapon);
};

class CBeam : public CBaseModelEntity {
public:
	DECLARE_SCHEMA_CLASS(CBeam);

	SCHEMA_FIELD(float, m_fWidth);
	SCHEMA_FIELD(Vector, m_vecEndPos);
};

class CBaseToggle : public CBaseModelEntity {
public:
	DECLARE_SCHEMA_CLASS(CBaseToggle);

	SCHEMA_FIELD(float, m_flWait);
};

class CBaseTrigger : public CBaseToggle {
public:
	DECLARE_SCHEMA_CLASS(CBaseTrigger);
};

class CModelPointEntity : public CBaseModelEntity {
public:
	DECLARE_SCHEMA_CLASS(CModelPointEntity);
};

class CPointWorldText : public CModelPointEntity {
public:
	DECLARE_SCHEMA_CLASS(CPointWorldText);

	SCHEMA_FIELD_STRING(m_messageText, 512);
	SCHEMA_FIELD_STRING(m_FontName, 64);
	SCHEMA_FIELD_STRING(m_BackgroundMaterialName, 64);
	SCHEMA_FIELD(bool, m_bEnabled);
	SCHEMA_FIELD(bool, m_bFullbright);
	SCHEMA_FIELD(float, m_flWorldUnitsPerPx);
	SCHEMA_FIELD(float, m_flFontSize);
	SCHEMA_FIELD(float, m_flDepthOffset);
	SCHEMA_FIELD(bool, m_bDrawBackground);
	SCHEMA_FIELD(float, m_flBackgroundBorderWidth);
	SCHEMA_FIELD(float, m_flBackgroundBorderHeight);
	SCHEMA_FIELD(float, m_flBackgroundWorldToUV);
	SCHEMA_FIELD(Color, m_Color);
	SCHEMA_FIELD(PointWorldTextJustifyHorizontal_t, m_nJustifyHorizontal);
	SCHEMA_FIELD(PointWorldTextJustifyVertical_t, m_nJustifyVertical);
	SCHEMA_FIELD(PointWorldTextReorientMode_t, m_nReorientMode);

public:
	void SetText(const char* msg) {
		AcceptInput("SetMessage", msg);
	}

	void Enable() {
		AcceptInput("Enable");
	}

	void Disable() {
		AcceptInput("Disable");
	}
};
