#pragma once

#include <pch.h>
#include <utils/typehelper.h>

// ======================== //
using SDKHookRet_Pre = bool;
using SDKHookRet_Post = void;

#define SDKHOOK_PRE(fnType)  RebindFunction_t<fnType, SDKHookRet_Pre>
#define SDKHOOK_POST(fnType) RebindFunction_t<fnType, SDKHookRet_Post>
// ======================== //

using HookTouch_t = void (*)(CBaseEntity* pSelf, CBaseEntity* pOther);
using HookTeleport_t = void (*)(CBaseEntity* pSelf, Vector* newPosition, QAngle* newAngles, Vector* newVelocity);
using HookUse_t = void (*)(CBaseEntity* pSelf, EntityInputData_t* pInput);

enum SDKHookType {
	SDKHook_StartTouch = 0,
	SDKHook_Touch,
	SDKHook_EndTouch,
	SDKHook_Teleport,
	SDKHook_Use,
	SDKHook_MAX_TYPE
};

template<SDKHookType T>
struct SDKHookBindings;

#define SDKHOOK_BIND(eType, fnType, gdName) \
	template<> \
	struct SDKHookBindings<eType> { \
		using HookType = fnType; \
		using Pre = SDKHOOK_PRE(HookType); \
		using Post = SDKHOOK_POST(HookType); \
		static constexpr const char* OffsetName = gdName; \
	};

SDKHOOK_BIND(SDKHook_StartTouch, HookTouch_t, "CBaseEntity::StartTouch");
SDKHOOK_BIND(SDKHook_Touch, HookTouch_t, "CBaseEntity::Touch");
SDKHOOK_BIND(SDKHook_EndTouch, HookTouch_t, "CBaseEntity::EndTouch");
SDKHOOK_BIND(SDKHook_Teleport, HookTeleport_t, "CBaseEntity::Teleport");
SDKHOOK_BIND(SDKHook_Use, HookUse_t, "CBaseEntity::Use");

namespace SDKHOOK {
	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Pre pListener);

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Post pListener);

	template<SDKHookType T>
	bool UnhookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Pre pListener);

	template<SDKHookType T>
	bool UnhookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Post pListener);
} // namespace SDKHOOK
