#include "cbaseentity.h"
#include <core/memory.h>

void CBaseEntity::AcceptInput(const char* pInputName, variant_t value, CEntityInstance* pActivator, CEntityInstance* pCaller) {
	MEM::CALL::CEntityInstance_AcceptInput(this, pInputName, pActivator, pCaller, &value, 0);
}

void CBaseEntity::DispatchSpawn(CEntityKeyValues* pInitKeyValue) {
	MEM::CALL::DispatchSpawn(this, pInitKeyValue);
}

void CBaseEntity::SetParent(CBaseEntity* pParent) {
	MEM::CALL::SetParent(this, pParent);
}

void CBaseEntity::SetName(const char* pszName, bool bCheckDuplicate) {
	if (bCheckDuplicate && !V_strcmp(this->m_pEntity->m_name.String(), pszName)) {
		return;
	}

	MEM::CALL::SetEntityName(this->m_pEntity, pszName);
}

const Vector& CBaseEntity::GetAbsOrigin() {
	auto pBodyComponent = m_CBodyComponent();
	if (!pBodyComponent) {
		return vec3_origin;
	}

	auto pNode = pBodyComponent->m_pSceneNode();
	if (!pNode) {
		return vec3_origin;
	}

	return pNode->m_vecAbsOrigin();
}

const Vector& CBaseEntity::GetOrigin() {
	auto pBodyComponent = m_CBodyComponent();
	if (!pBodyComponent) {
		return vec3_origin;
	}

	auto pNode = pBodyComponent->m_pSceneNode();
	if (!pNode) {
		return vec3_origin;
	}

	return pNode->m_vecOrigin();
}

const QAngle& CBaseEntity::GetAbsAngles() {
	auto pBodyComponent = m_CBodyComponent();
	if (!pBodyComponent) {
		return vec3_angle;
	}

	auto pNode = pBodyComponent->m_pSceneNode();
	if (!pNode) {
		return vec3_angle;
	}

	return pNode->m_angAbsRotation();
}
