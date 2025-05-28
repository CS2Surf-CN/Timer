#include "sdkhook.h"
#include <core/memory.h>
#include <utils/utils.h>
#include <sdk/entity/ccsplayercontroller.h>

#include <unordered_set>
#include <utility>
#include <list>

class SDKHookManager : CCoreForward {
private:
	virtual void OnEntityDeleted(CEntityInstance* pEntity) override;

public:
	bool IsVMTHooked(void* pVtable);
	bool IsVMTHooked(void* pVtable, uint32_t iOffset);
	void HookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pInternalCallback, void* pListener);
	void UnhookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pInternalCallback, void* pListener);
	void UnhookVMT(CBaseEntity* pEnt);

public:
	// offset -> count
	using VMTHookCounter_t = std::unordered_map<uint32_t, uint32_t>;
	// vtable -> counter
	std::unordered_map<void*, VMTHookCounter_t> m_umVMTHooked {};

	// {ehandle, pListener}
	using VMTHookListenerContext_t = std::pair<CEntityHandle, void*>;
	using VMTHookListenerList_t = std::list<VMTHookListenerContext_t>;
	// [type][pre or post]::vtable -> listener list
	std::unordered_map<void*, VMTHookListenerList_t> m_umSDKHooksListeners[SDKHookType::SDKHook_MAX_TYPE][2] {};

	// [type]::vtable -> internal templated-callback
	std::unordered_map<void*, void*> m_umSDKHookCallbacks[SDKHookType::SDKHook_MAX_TYPE] {};

	// [type]::vtable -> trampoline
	std::unordered_map<void*, void*> m_umSDKHookTrampolines[SDKHookType::SDKHook_MAX_TYPE] {};

	std::unordered_map<std::string, uint32_t> m_umVMTOffsets {};
} g_SDKHookManager;

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

	static auto HookInstantiator = (ForceInstantiation(std::make_integer_sequence<int, SDKHookType::SDKHook_MAX_TYPE> {}), 0);

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

	template<int... Is>
	static void UninstallHookAuto(SDKHookType type, CBaseEntity* pEnt, void* pListener, bool post, std::integer_sequence<int, Is...>) {
		((type == static_cast<SDKHookType>(Is) ? (SDKHOOK::UninstallHook<static_cast<SDKHookType>(Is)>(pEnt, pListener, post), true) : false) || ...);
	}

	static void UninstallHookRT(SDKHookType type, CBaseEntity* pEnt, void* pListener, bool post) {
		UninstallHookAuto(type, pEnt, pListener, post, std::make_integer_sequence<int, SDKHookType::SDKHook_MAX_TYPE> {});
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

void SDKHookManager::HookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pInternalCallback, void* pListener) {
	if (!m_umVMTOffsets.contains(gdOffsetName)) {
		m_umVMTOffsets[gdOffsetName] = GAMEDATA::GetOffset(gdOffsetName);
	}

	void* pVtable = *(void**)pEnt;
	auto iOffset = m_umVMTOffsets.at(gdOffsetName);
	if (!IsVMTHooked(pVtable, iOffset)) {
		MEM::AddVMTHook(pVtable, iOffset, pInternalCallback, m_umSDKHookTrampolines[type][pVtable]);
		m_umSDKHookCallbacks[type][pVtable] = pInternalCallback;
		m_umVMTHooked[pVtable][iOffset] = 1;
	} else {
		m_umVMTHooked[pVtable][iOffset]++;
	}

	auto& listenerList = m_umSDKHooksListeners[type][post][pVtable];
	auto hEnt = pEnt->GetRefEHandle();
	auto same_ctx = std::ranges::find_if(listenerList, [hEnt, pListener](const auto& pair) { return pair.first == hEnt && pair.second == pListener; });
	if (same_ctx != listenerList.end()) {
		SDK_ASSERT(false); // design error
		Plat_FatalErrorFunc("Hooking the same entity and listener is not allowed");
	}

	listenerList.emplace_back(hEnt, pListener);
}

void SDKHookManager::UnhookVMT(CBaseEntity* pEnt, std::string gdOffsetName, SDKHookType type, bool post, void* pInternalCallback, void* pListener) {
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
	auto hEnt = pEnt->GetRefEHandle();
	auto ctx_it = std::ranges::find_if(listenerList, [hEnt, pListener](const auto& pair) { return pair.first == hEnt && pair.second == pListener; });
	if (ctx_it == listenerList.end()) {
		SDK_ASSERT(false);
		return;
	}

	listenerList.erase(ctx_it);

	auto& counter = m_umVMTHooked[pVtable][iOffset];
	counter--;
	if (!counter) {
		MEM::RemoveVMTHook(pVtable, iOffset, pInternalCallback, m_umSDKHookTrampolines[type][pVtable]);
		m_umSDKHookTrampolines[type].erase(pVtable);
		m_umSDKHookCallbacks[type].erase(pVtable);
		m_umSDKHooksListeners[type][post].erase(pVtable);
		m_umVMTHooked[pVtable].erase(iOffset);
	}

	if (m_umVMTHooked[pVtable].empty()) {
		m_umVMTHooked.erase(pVtable);
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

	auto hTargetEnt = pEnt->GetRefEHandle();
	for (int type = 0; type < SDKHookType::SDKHook_MAX_TYPE; type++) {
		if (m_umSDKHookCallbacks[type].contains(pVtable)) {
			for (int i = 0; i <= 1; i++) {
				if (m_umSDKHooksListeners[type][i].contains(pVtable)) {
					for (const auto& [hEnt, pListener] : m_umSDKHooksListeners[type][i].at(pVtable)) {
						if (hEnt == hTargetEnt) {
							SDKHOOK::UninstallHookRT(static_cast<SDKHookType>(type), pEnt, pListener, i);
						}
					}
				}
			}
		}
	}
}

void SDKHookManager::OnEntityDeleted(CEntityInstance* pEntity) {
	UnhookVMT(static_cast<CBaseEntity*>(pEntity));
}
