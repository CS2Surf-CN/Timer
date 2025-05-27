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

		volatile PreFunc hook_pre_ptr = static_cast<PreFunc>(&SDKHOOK::HookEntity<T>);
		volatile PostFunc hook_post_ptr = static_cast<PostFunc>(&SDKHOOK::HookEntity<T>);
		volatile PreFunc unhook_pre_ptr = static_cast<PreFunc>(&SDKHOOK::UnhookEntity<T>);
		volatile PostFunc unhook_post_ptr = static_cast<PostFunc>(&SDKHOOK::UnhookEntity<T>);
	}

	template<int... Is>
	void ForceInstantiation(std::integer_sequence<int, Is...>) {
		int dummy[] = {(InstantiateHookType<static_cast<SDKHookType>(Is)>(), 0)...};
	}

	static auto HookInstantiator = (ForceInstantiation(std::make_integer_sequence<int, SDKHookType::MAX_TYPE> {}), 0);

	template<SDKHookType T>
	auto HookEntityT(CBaseEntity* pSelf, auto... args) {
		using Traits = SDKHookBindings<T>;
		using HookType_t = typename Traits::HookType;

		void* vtable = *(void**)pSelf;
		HookType_t pTrampoline = (HookType_t)g_SDKHookManager.m_umSDKHookTrampolines[T][vtable];
		SDK_ASSERT(pTrampoline);

		bool block = false;
		const auto& preHooks = g_SDKHookManager.m_umSDKHooksListeners[T][0][vtable];
		for (const auto& [eHandle, pFnPre] : preHooks) {
			if (eHandle == pSelf->GetRefEHandle() && !reinterpret_cast<SDKHOOK_PRE(HookType_t)*>(pFnPre)(pSelf, args...)) {
				block = true;
			}
		}
		if (block) {
			return;
		}

		pTrampoline(pSelf, args...);

		const auto& postHooks = g_SDKHookManager.m_umSDKHooksListeners[T][1][vtable];
		for (const auto& [eHandle, pFnPost] : postHooks) {
			if (eHandle == pSelf->GetRefEHandle()) {
				reinterpret_cast<SDKHOOK_POST(HookType_t)*>(pFnPost)(pSelf, args...);
			}
		}
	};

	template<SDKHookType T>
	static void InstallHook(CBaseEntity* pEnt, void* pListener, bool post) {
		using Traits = SDKHookBindings<T>;
		using HookType_t = typename Traits::HookType;

		HookType_t pHook = &HookEntityT<T>;
		g_SDKHookManager.HookVMT(pEnt, Traits::OffsetName, T, post, (void*)pHook, pListener);
	}

	template<SDKHookType T>
	static void UninstallHook(CBaseEntity* pEnt, void* pListener, bool post) {
		using Traits = SDKHookBindings<T>;
		using HookType_t = typename Traits::HookType;

		HookType_t pHook = &HookEntityT<T>;
		g_SDKHookManager.UnhookVMT(pEnt, Traits::OffsetName, T, post, (void*)pHook, pListener);
	}

	template<SDKHookType I = (SDKHookType)0>
	static void UninstallHookRT(SDKHookType type, CBaseEntity* pEnt, void* pListener, bool post) {
		if constexpr (I < SDKHookType::MAX_TYPE) {
			if (type == I) {
				SDKHOOK::UninstallHook<static_cast<SDKHookType>(I)>(pEnt, pListener, post);
			} else {
				SDKHOOK::UninstallHookRT<static_cast<SDKHookType>(I + 1)>(type, pEnt, pListener, post);
			}
		}
	}

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Pre pListener) {
		InstallHook<T>(pEnt, (void*)pListener, false);
		return true;
	}

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Post pListener) {
		InstallHook<T>(pEnt, (void*)pListener, true);
		return true;
	}

	template<SDKHookType T>
	bool UnhookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Pre pListener) {
		UninstallHook<T>(pEnt, (void*)pListener, false);
		return true;
	}

	template<SDKHookType T>
	bool UnhookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Post pListener) {
		UninstallHook<T>(pEnt, (void*)pListener, true);
		return true;
	}
} // namespace SDKHOOK

bool SDKHookManager::IsVMTHooked(void* pVtable) {
	return m_umVMTHooked.find(pVtable) != m_umVMTHooked.end();
}

bool SDKHookManager::IsVMTHooked(void* pVtable, uint32_t iOffset) {
	if (auto it = m_umVMTHooked.find(pVtable); it != m_umVMTHooked.end() && it->second.contains(iOffset)) {
		return it->second.at(iOffset) > 0;
	}

	return false;
}

void SDKHookManager::HookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pCallback, void* pListener) {
	if (!m_umVMTOffsets.contains(gdOffsetName)) {
		m_umVMTOffsets[gdOffsetName] = GAMEDATA::GetOffset(gdOffsetName);
	}

	void* pVtable = *(void**)pEnt;
	auto iOffset = m_umVMTOffsets.at(gdOffsetName);
	if (!IsVMTHooked(pVtable, iOffset)) {
		MEM::AddVMTHook(pVtable, iOffset, pCallback, m_umSDKHookTrampolines[type][pVtable]);
		m_umSDKHookOriginals[type][pVtable] = pCallback;
		m_umVMTHooked[pVtable][iOffset] = 1;
	} else {
		m_umVMTHooked[pVtable][iOffset]++;
	}

	auto& listenerList = m_umSDKHooksListeners[type][post][pVtable];
	auto hEnt = pEnt->GetRefEHandle();
	auto same_ctx = std::ranges::find_if(listenerList, [hEnt, pListener](const auto& pair) { return pair.first == hEnt && pair.second == pListener; });
	if (same_ctx != listenerList.end()) {
		SDK_ASSERT(false); // design error
	}

	listenerList.emplace_back(hEnt, pListener);
}

void SDKHookManager::UnhookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pCallback, void* pListener) {
	if (!m_umVMTOffsets.contains(gdOffsetName)) {
		m_umVMTOffsets[gdOffsetName] = GAMEDATA::GetOffset(gdOffsetName);
	}

	void* pVtable = *(void**)pEnt;
	auto iOffset = m_umVMTOffsets.at(gdOffsetName);
	if (!IsVMTHooked(pVtable, iOffset)) {
		return;
	}

	if (!m_umSDKHooksListeners[type][post].contains(pVtable)) {
		SDK_ASSERT(false);
		return;
	}

	auto& listenerList = m_umSDKHooksListeners[type][post].at(pVtable);
	auto ctx_it = std::ranges::find_if(listenerList, [pListener](const auto& pair) { return pair.second == pListener; });
	if (ctx_it == listenerList.end()) {
		SDK_ASSERT(false);
		return;
	}

	if (ctx_it->first != pEnt->GetRefEHandle() || ctx_it->second != pListener) {
		return;
	}

	listenerList.erase(ctx_it);

	auto& counter = m_umVMTHooked[pVtable][iOffset];
	counter--;
	if (!counter) {
		MEM::RemoveVMTHook(pVtable, iOffset, pCallback, m_umSDKHookTrampolines[type][pVtable]);
		m_umSDKHookTrampolines[type].erase(pVtable);
		m_umSDKHookOriginals[type].erase(pVtable);
		m_umVMTHooked.erase(pVtable);
		m_umSDKHooksListeners[type][post].erase(pVtable);
	}
}

void SDKHookManager::UnhookVMT(CBaseEntity* pEnt) {
	if (!pEnt) {
		return;
	}

	void* pVtable = *(void**)pEnt;
	if (!IsVMTHooked(pVtable)) {
		return;
	}

	for (int type = 0; type < SDKHookType::MAX_TYPE; type++) {
		if (m_umSDKHookOriginals[type].contains(pVtable)) {
			for (const auto& ctx : m_umSDKHooksListeners[type][0][pVtable]) {
				SDKHOOK::UninstallHookRT(static_cast<SDKHookType>(type), pEnt, ctx.second, false);
			}
			for (const auto& ctx : m_umSDKHooksListeners[type][1][pVtable]) {
				SDKHOOK::UninstallHookRT(static_cast<SDKHookType>(type), pEnt, ctx.second, true);
			}
		}
	}
}

void SDKHookManager::OnEntityDeleted(CEntityInstance* pEntity) {
	UnhookVMT(dynamic_cast<CBaseEntity*>(pEntity));
}
