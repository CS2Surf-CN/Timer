#include "sdkhook.h"
#include <core/memory.h>

#include <unordered_set>
#include <utility>
#include <list>

SDKHookManager g_SDKHookManager;

namespace SDKHOOK {
	template<SDKHookType T>
	void InstantiateHookType() {
		using PreFunc = bool (*)(CBaseEntity*, typename SDKHookBindings<T>::Pre);
		using PostFunc = bool (*)(CBaseEntity*, typename SDKHookBindings<T>::Post);

		volatile PreFunc pre_ptr = static_cast<PreFunc>(&SDKHOOK::HookEntity<T>);
		volatile PostFunc post_ptr = static_cast<PostFunc>(&SDKHOOK::HookEntity<T>);
	}

	template<int... Is>
	void ForceInstantiation(std::integer_sequence<int, Is...>) {
		int dummy[] = {(InstantiateHookType<static_cast<SDKHookType>(Is)>(), 0)...};
	}

	static auto HookInstantiator = (ForceInstantiation(std::make_integer_sequence<int, SDKHookType::MAX_TYPE> {}), 0);

	template<SDKHookType T>
	static void InstallHook(CBaseEntity* pEnt, void* pCallback, bool post) {
		using Traits = SDKHookBindings<T>;
		using HookType_t = typename Traits::HookType;

		HookType_t pHook = [](CBaseEntity* pSelf, auto... args) {
			void* vtable = *(void**)pSelf;
			HookType_t pTrampoline = (HookType_t)g_SDKHookManager.m_umSDKHookTrampolines[T][vtable];
			SDK_ASSERT(pTrampoline);

			bool block = false;
			const auto& preHooks = g_SDKHookManager.m_umSDKHooks[T][0][vtable];
			for (const auto& pFnPre : preHooks) {
				if (!reinterpret_cast<SDKHOOK_PRE(HookType_t)*>(pFnPre)(pSelf, args...)) {
					block = true;
				}
			}
			if (block) {
				return;
			}

			pTrampoline(pSelf, args...);

			const auto& postHooks = g_SDKHookManager.m_umSDKHooks[T][1][vtable];
			for (const auto& pFnPost : postHooks) {
				reinterpret_cast<SDKHOOK_POST(HookType_t)*>(pFnPost)(pSelf, args...);
			}
		};

		g_SDKHookManager.AddVMTHook(pEnt, Traits::OffsetName, T, post, pHook, pCallback);
	}

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Pre pCallback) {
		InstallHook<T>(pEnt, (void*)pCallback, false);
		return true;
	}

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Post pCallback) {
		InstallHook<T>(pEnt, (void*)pCallback, true);
		return true;
	}
} // namespace SDKHOOK

bool SDKHookManager::IsVMTHooked(void* pVtable, uint32_t iOffset) {
	if (auto it = m_umVMTHooked.find(pVtable); it != m_umVMTHooked.end()) {
		return it->second.contains(iOffset);
	}
	return false;
}

void SDKHookManager::AddVMTHook(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pCallback, void* pListener) {
	if (!m_umVMTOffsets.contains(gdOffsetName)) {
		m_umVMTOffsets[gdOffsetName] = GAMEDATA::GetOffset(gdOffsetName);
	}

	void* pVtable = *(void**)pEnt;
	auto iOffset = m_umVMTOffsets.at(gdOffsetName);
	MEM::AddVMTHook(pEnt, iOffset, pCallback, m_umSDKHookTrampolines[type][pVtable]);

	auto& hookCtx = m_umSDKHooks[type][post][pVtable];
	if (std::find(hookCtx.begin(), hookCtx.end(), pListener) == hookCtx.end()) {
		hookCtx.emplace_back(pListener);
	}
}

void SDKHookManager::OnEntityDeleted(CEntityInstance* pEntity) {
	
}
