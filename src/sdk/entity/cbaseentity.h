#pragma once

#include <ehandle.h>
#include <core/gamedata.h>
#include <sdk/schema.h>
#include <libmem/libmem_virtual.h>
#include <sdk/entity/ccollisionproperty.h>

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

	SCHEMA_FIELD(CNetworkedQuantizedFloat, m_vecX);
	SCHEMA_FIELD(CNetworkedQuantizedFloat, m_vecY);
	SCHEMA_FIELD(CNetworkedQuantizedFloat, m_vecZ);
};

class CNetworkVelocityVector {
public:
	DECLARE_SCHEMA_CLASS_INLINE(CNetworkVelocityVector);

	SCHEMA_FIELD(CNetworkedQuantizedFloat, m_vecX);
	SCHEMA_FIELD(CNetworkedQuantizedFloat, m_vecY);
	SCHEMA_FIELD(CNetworkedQuantizedFloat, m_vecZ);
};

class CGameSceneNode {
public:
	DECLARE_SCHEMA_CLASS(CGameSceneNode)

	SCHEMA_FIELD(CEntityInstance*, m_pOwner);
	SCHEMA_FIELD(CGameSceneNode*, m_pParent);
	SCHEMA_FIELD(CGameSceneNode*, m_pChild);
	SCHEMA_FIELD(Vector, m_vecOrigin);
	SCHEMA_FIELD(QAngle, m_angRotation);
	SCHEMA_FIELD(QAngle, m_angAbsRotation);
	SCHEMA_FIELD(float, m_flScale);
	SCHEMA_FIELD(float, m_flAbsScale);
	SCHEMA_FIELD(Vector, m_vecAbsOrigin);
	SCHEMA_FIELD(Vector, m_vRenderOrigin);
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
	SCHEMA_FIELD(float, m_flGravityScale);
	SCHEMA_FIELD(float, m_flWaterLevel);
	SCHEMA_FIELD(int, m_fEffects);
	SCHEMA_FIELD(CHandle<CBaseEntity>, m_hOwnerEntity);

	int entindex() {
		return m_pEntity->m_EHandle.GetEntryIndex();
	}

	bool IsPawn() {
		return CALL_VIRTUAL(bool, GAMEDATA::GetOffset("IsEntityPawn"), this);
	}

	bool IsController() {
		return CALL_VIRTUAL(bool, GAMEDATA::GetOffset("IsEntityController"), this);
	}

	bool IsAlive() {
		return this->m_lifeState() == LIFE_ALIVE;
	}

	void SetMoveType(MoveType_t movetype) {
		this->m_MoveType(movetype);
		this->m_nActualMoveType(movetype);
	}

	void CollisionRulesChanged() {
		CALL_VIRTUAL(void, GAMEDATA::GetOffset("CollisionRulesChanged"), this);
	}

	int GetTeam() {
		return m_iTeamNum();
	}

	void StartTouch(CBaseEntity* pOther) {
		CALL_VIRTUAL(bool, GAMEDATA::GetOffset("StartTouch"), this, pOther);
	}

	void Touch(CBaseEntity* pOther) {
		CALL_VIRTUAL(bool, GAMEDATA::GetOffset("Touch"), this, pOther);
	}

	void EndTouch(CBaseEntity* pOther) {
		CALL_VIRTUAL(bool, GAMEDATA::GetOffset("EndTouch"), this, pOther);
	}

	void Teleport(const Vector* newPosition, const QAngle* newAngles, const Vector* newVelocity) {
		CALL_VIRTUAL(bool, GAMEDATA::GetOffset("Teleport"), this, newPosition, newAngles, newVelocity);
	}

	void Kill() {
		this->AcceptInput("kill");
	}

	void AcceptInput(const char* pInputName, variant_t value = variant_t(""), CEntityInstance* pActivator = nullptr,
					 CEntityInstance* pCaller = nullptr);

	void DispatchSpawn(CEntityKeyValues* pInitKeyValue = nullptr);
	void SetParent(CBaseEntity* pParent);

	Vector& GetAbsOrigin();
	Vector& GetOrigin();
	QAngle& GetAbsAngles();
};

class CBaseModelEntity : public CBaseEntity {
public:
	DECLARE_SCHEMA_CLASS(CBaseModelEntity);

	SCHEMA_FIELD_POINTER(CCollisionProperty, m_Collision);
	SCHEMA_FIELD(Color, m_clrRender);
	SCHEMA_FIELD(RenderMode_t, m_nRenderMode);
	SCHEMA_FIELD(float, m_fadeMinDist);
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

	SCHEMA_FIELD_POINTER(char, m_messageText);

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
