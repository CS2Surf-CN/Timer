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
	SDKHook_StartTouch,
	SDKHook_Touch,
	SDKHook_EndTouch,
	SDKHook_Teleport,
	SDKHook_Use,
	MAX_TYPE
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

class SDKHookManager : CCoreForward {
private:
	virtual void OnEntityDeleted(CEntityInstance* pEntity) override;

public:
	bool IsVMTHooked(void* pVtable, uint32_t iOffset);
	void AddVMTHook(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pCallback, void* pListener);
	void RemoveVMTHook(CBaseEntity* pEnt, SDKHookType type);

public:
	// vtable -> hooked vfuncs by offset
	std::unordered_map<void*, std::unordered_set<uint32_t>> m_umVMTHooked {};

	// [type][pre or post]::vtable -> registered contexts(ehandle, pCallbackList)
	using HookRegisterCtx = std::pair<CEntityHandle, std::list<void*>>;
	std::unordered_map<void*, std::list<void*>> m_umSDKHooks[SDKHookType::MAX_TYPE][2] {};

	// [type]::vtable -> trampoline
	std::unordered_map<void*, void*> m_umSDKHookTrampolines[SDKHookType::MAX_TYPE] {};

	std::unordered_map<std::string, uint32_t> m_umVMTOffsets {};
};

namespace SDKHOOK {
	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Pre pCallback);

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Post pCallback);
} // namespace SDKHOOK
