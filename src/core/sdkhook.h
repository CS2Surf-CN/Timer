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
	bool IsVMTHooked(void* pVtable);
	bool IsVMTHooked(void* pVtable, uint32_t iOffset);
	void HookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pCallback, void* pListener);
	void UnhookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pCallback, void* pListener);
	void UnhookVMT(CBaseEntity* pEnt);

public:
	// offset -> count
	using VMTHookCounter_t = std::unordered_map<uint32_t, uint32_t>;
	// vtable -> counter
	std::unordered_map<void*, VMTHookCounter_t> m_umVMTHooked {};

	// {ehandle, pCallback}
	using VMTHookListenerContext_t = std::pair<CEntityHandle, void*>;
	using VMTHookListenerList_t = std::list<VMTHookListenerContext_t>;
	// [type][pre or post]::vtable -> listener list
	std::unordered_map<void*, VMTHookListenerList_t> m_umSDKHooksListeners[SDKHookType::MAX_TYPE][2] {};

	// [type]::vtable -> original
	std::unordered_map<void*, void*> m_umSDKHookOriginals[SDKHookType::MAX_TYPE] {};

	// [type]::vtable -> trampoline
	std::unordered_map<void*, void*> m_umSDKHookTrampolines[SDKHookType::MAX_TYPE] {};

	std::unordered_map<std::string, uint32_t> m_umVMTOffsets {};
};

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
