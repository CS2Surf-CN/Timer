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

Vector& CBaseEntity::GetAbsOrigin() {
	static Vector null(0.0f, 0.0f, 0.0f);
	auto pBodyComponent = m_CBodyComponent();
	if (!pBodyComponent) {
		return null;
	}

	auto pNode = pBodyComponent->m_pSceneNode();
	if (!pNode) {
		return null;
	}

	return pNode->m_vecAbsOrigin();
}

Vector& CBaseEntity::GetOrigin() {
	static Vector null(0.0f, 0.0f, 0.0f);
	auto pBodyComponent = m_CBodyComponent();
	if (!pBodyComponent) {
		return null;
	}

	auto pNode = pBodyComponent->m_pSceneNode();
	if (!pNode) {
		return null;
	}

	return pNode->m_vecOrigin();
}

QAngle& CBaseEntity::GetAbsAngles() {
	static QAngle null(0.0f, 0.0f, 0.0f);
	auto pBodyComponent = m_CBodyComponent();
	if (!pBodyComponent) {
		return null;
	}

	auto pNode = pBodyComponent->m_pSceneNode();
	if (!pNode) {
		return null;
	}

	return pNode->m_angAbsRotation();
}
