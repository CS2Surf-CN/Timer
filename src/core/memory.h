#pragma once

#include <pch.h>

#include <sdk/common.h>
#include <sdk/datatypes.h>
#include <sdk/server/weapon.h>
#include <libmem/libmem.hpp>
#include <libmem/libmem_helper.h>
#include <libmodule/module.h>
#include <polyhook2/Detour/NatDetour.hpp>
#include <utils/vtablehelper.h>

#include <list>

class CBasePlayerController;
class CCSPlayerController;
class CCSPlayerPawn;
class CEntityInstance;
class CEntityKeyValues;
class CBaseTrigger;

class GameSessionConfiguration_t {};

namespace MEM {
	namespace CALL {
		void SwitchTeam(CCSPlayerController* controller, int team);
		void SetPawn(CBasePlayerController* controller, CCSPlayerPawn* pawn, bool a3, bool a4, bool a5);
		IGameEventListener2* GetLegacyGameEventListener(CPlayerSlot slot);
		bool TraceShape(const Ray_t& ray, const Vector& vecStart, const Vector& vecEnd, const CTraceFilter& filter, CGameTrace* tr);
		void TracePlayerBBox(const Vector& start, const Vector& end, const bbox_t& bounds, CTraceFilter* filter, trace_t& pm);
		void InitPlayerMovementTraceFilter(CTraceFilterPlayerMovementCS& pFilter, CEntityInstance* pHandleEntity, uint64 interactWith, int collisionGroup);
		void SnapViewAngles(CBasePlayerPawn* pawn, const QAngle& angle);
		void CEntityInstance_AcceptInput(CEntityInstance* pEnt, const char* pInputName, CEntityInstance* pActivator, CEntityInstance* pCaller, variant_t* value, int nOutputID);
		CBaseEntity* CreateEntityByName(const char* pszName);
		void DispatchSpawn(CBaseEntity* pEnt, CEntityKeyValues* pInitKeyValue);
		CBaseTrigger* CreateAABBTrigger(const Vector& center, const Vector& mins, const Vector& maxs);
		void SetParent(CBaseEntity* pEnt, CBaseEntity* pParent);
		void SetEntityName(CEntityIdentity* pEnt, const char* pszName);
		SndOpEventGuid_t EmitSound(IRecipientFilter& filter, CEntityIndex ent, const EmitSound_t& params);
		bool BotAddCommand(int team, bool isFromConsole = false, const char* profileName = nullptr, CSWeaponType weaponType = WEAPONTYPE_UNKNOWN, int difficulty = 0);
	} // namespace CALL

	namespace MODULE {
		inline std::shared_ptr<libmodule::CModule> engine;
		inline std::shared_ptr<libmodule::CModule> tier0;
		inline std::shared_ptr<libmodule::CModule> server;
		inline std::shared_ptr<libmodule::CModule> schemasystem;
		inline std::shared_ptr<libmodule::CModule> steamnetworkingsockets;

		void Setup();
	} // namespace MODULE

	namespace TRAMPOLINE {
		inline void* g_fnApplyGameSettings;
		inline void* g_fnGameFrame;
		inline void* g_fnClientConnect;
		inline void* g_fnClientConnected;
		inline void* g_fnClientFullyConnect;
		inline void* g_fnClientPutInServer;
		inline void* g_fnClientActive;
		inline void* g_fnClientDisconnect;
		inline void* g_fnClientVoice;
		inline void* g_fnClientCommand;
		inline void* g_fnCreateGameEvent;
		inline void* g_fnFireGameEvent;
		inline void* g_fnStartupServer;
		inline void* g_fnDispatchConCommand;
		inline void* g_fnPostEventAbstract;
		inline void* g_fnServerGamePostSimulate;
		inline void* g_fnActivateServer;
		inline void* g_fnWeaponDrop;
		inline void* g_fnTakeDamage;
		inline void* g_fnClientSendSnapshotBefore;
		inline void* g_fnSetObserverTarget;
	} // namespace TRAMPOLINE

	void SetupHooks();

	class CHookManager {
	public:
		// [{pDetour, pCallback}]
		std::list<std::pair<std::unique_ptr<PLH::NatDetour>, std::uintptr_t>> m_DetourList;

		// [{pVMT, pCallback}]
		std::list<std::pair<std::unique_ptr<libmem::Vmt>, std::uintptr_t>> m_VMTHookList;
	};

	extern CHookManager* GetHookManager();

	template<typename TOriginal, typename TCallback, typename TTram>
	bool AddDetour(TOriginal pOriginal, TCallback pCallback, TTram& pTrampoline) {
		auto pDetour = std::make_unique<PLH::NatDetour>((uint64_t)pOriginal, (uint64_t)pCallback, (uint64_t*)&pTrampoline);
		if (!pDetour->hook()) {
			SDK_ASSERT(false);
			return false;
		}

		GetHookManager()->m_DetourList.emplace_back(std::pair {std::move(pDetour), (uintptr_t)pCallback});
		return true;
	}

	template<typename TOriginal, typename TTram>
	bool RemoveDetour(TOriginal pCallback, TTram& pTrampoline) {
		auto pTarget = reinterpret_cast<uintptr_t>(pCallback);
		auto& pDetourList = GetHookManager()->m_DetourList;
		auto it = std::ranges::find_if(pDetourList, [pTarget](const auto& pair) { return pair.second == pTarget; });

		if (it != pDetourList.end()) {
			it->first->unHook();
			pDetourList.erase(it);
			pTrampoline = nullptr;
			return true;
		}

		return false;
	}

	template<typename TCallback, typename TTram>
	bool AddVMTHook(void* pVtable, uint32_t vfnIndex, TCallback pCallback, TTram& pTrampoline) {
#ifdef SDK_DEBUG
		auto& hooklist = GetHookManager()->m_VMTHookList;
		for (const auto& [vmt, _] : hooklist) {
			auto vtb = *(libmem::Address**)vmt->Convert();
			void* pCurrentFn = *reinterpret_cast<void**>(vtb + vfnIndex);
			if (pCurrentFn == (void*)pCallback && vtb == pVtable) {
				SDK_ASSERT(false);
			}
		}
#endif

		std::unique_ptr<libmem::Vmt> pVMT = std::make_unique<libmem::Vmt>(static_cast<libmem::Address*>(pVtable));
		pTrampoline = pVMT->GetOriginal<TTram>(vfnIndex);
		pVMT->Hook(vfnIndex, (libmem::Address)pCallback);

		GetHookManager()->m_VMTHookList.emplace_back(std::pair {std::move(pVMT), (uintptr_t)pCallback});

		return true;
	}

	template<typename TCallback, typename TTram>
	bool AddVMTInstanceHook(void* pInstance, uint32_t vfnIndex, TCallback pCallback, TTram& pTrampoline) {
		void* pVtable = *(void**)pInstance;
		return MEM::AddVMTHook(pVtable, vfnIndex, pCallback, pTrampoline);
	}

	template<typename TCallback, typename TTram>
	bool AddVMTHookEx(libmodule::CModule* pModule, std::string sClassName, uint32_t vfnIndex, TCallback pCallback, TTram& pTrampoline, bool bDetour = false) {
		if (!pModule) {
			return false;
		}

		void* pVtable = pModule->GetVirtualTableByName(sClassName);
		if (!pVtable) {
			return false;
		}

		if (bDetour) {
			libmem::Vmt vmt((libmem::Address*)pVtable);
			auto pVfunc = vmt.GetOriginal(vfnIndex);
			MEM::AddDetour(pVfunc, pCallback, pTrampoline);
		} else {
			MEM::AddVMTHook(pVtable, vfnIndex, pCallback, pTrampoline);
		}

		return true;
	}

	template<typename TCallback, typename TTram>
	bool RemoveVMTHook(void* pVtable, uint32_t vfnIndex, TCallback pCallback, TTram& pTrampoline) {
		auto pTarget = reinterpret_cast<uintptr_t>(pCallback);
		auto& pVMTHookList = GetHookManager()->m_VMTHookList;
		auto it = std::ranges::find_if(pVMTHookList, [pVtable, pTarget](const auto& pair) { return (void*)pair.first->GetVTable() == pVtable && pair.second == pTarget; });

		if (it != pVMTHookList.end()) {
			it->first->Unhook(vfnIndex);
			pVMTHookList.erase(it);
			pTrampoline = nullptr;
			return true;
		}

		return false;
	}

	template<typename TCallback, typename TTram>
	bool RemoveVMTInstanceHook(void* pInstance, uint32_t vfnIndex, TCallback pCallback, TTram& pTrampoline) {
		void* pVtable = *(void**)pInstance;
		return MEM::RemoveVMTHook(pVtable, vfnIndex, pCallback, pTrampoline);
	}

	template<typename T = void, typename... Args>
	typename std::enable_if<!std::is_void<T>::value, T>::type SDKCall(void* pAddress, Args... args) {
		auto pFn = reinterpret_cast<T (*)(Args...)>(pAddress);
		SDK_ASSERT((uintptr_t)pFn);
		return pFn(args...);
	}

	template<typename T = void, typename... Args>
	typename std::enable_if<std::is_void<T>::value, void>::type SDKCall(void* pAddress, Args... args) {
		auto pFn = reinterpret_cast<void (*)(Args...)>(pAddress);
		SDK_ASSERT((uintptr_t)pFn);
		pFn(args...);
	}

	template<typename T = void*>
	T GetVMethod(uint32_t uIndex, void* pInstance) {
		if (!pInstance) {
			printf("vmt::GetVMethod failed: invalid instance pointer\n");
			return T();
		}

		void** vtable = *static_cast<void***>(pInstance);
		if (!vtable) {
			printf("vmt::GetVMethod failed: invalid vtable pointer\n");
			return T();
		}

		return reinterpret_cast<T>(vtable[uIndex]);
	}

	template<typename Ret = void, typename T, typename... Args>
	inline Ret CallVirtual(uint32_t uIndex, T pClass, Args... args) {
		auto func_ptr = GetVMethod(uIndex, pClass);
		if (!func_ptr) {
			printf("vmt::CallVirtual failed: invalid function pointer\n");
			return Ret();
		}

#ifdef _WIN32
		class VType {};

		union VConverter {
			VConverter(void* _ptr) : ptr(_ptr) {}

			void* ptr;
			Ret (__thiscall VType::*fn)(Args...);
		} v(func_ptr);

		return ((VType*)pClass->*v.fn)(args...);
#else
		return reinterpret_cast<Ret (*)(T, Args...)>(func_ptr)(pClass, args...);
#endif
	}

	template<typename T, typename... Args>
	auto GetFn(void* pAddress, Args... args) {
		return reinterpret_cast<T (*)(Args...)>(pAddress);
	}

	template<size_t nOffset = 0>
	inline void PatchNOP(void* pAddress, size_t nLen) {
		uint8_t* adr_patch = static_cast<uint8_t*>(pAddress) + nOffset;
		auto old_mem_prot = libmem::ProtMemory((libmem::Address)adr_patch, nLen, libmem::Prot::XRW);
		if (old_mem_prot.has_value()) {
			libmem::SetMemory((libmem::Address)adr_patch, 0x90, nLen);
			libmem::ProtMemory((libmem::Address)adr_patch, nLen, old_mem_prot.value());
		}
	}

	template<size_t nOffset = 0, typename... Args>
	inline void PatchAddress(void* pAddress, Args... args) {
		const uint8_t bytes[] = {static_cast<uint8_t>(args)...};
		uint8_t* adr_patch = static_cast<uint8_t*>(pAddress) + nOffset;
		const size_t patch_size = sizeof...(args);
		auto old_mem_prot = libmem::ProtMemory((libmem::Address)adr_patch, patch_size, libmem::Prot::XRW);
		for (size_t i = 0; i < patch_size; ++i) {
			if (old_mem_prot.has_value()) {
				adr_patch[i] = bytes[i];
			}
		}
		libmem::ProtMemory((libmem::Address)adr_patch, patch_size, old_mem_prot.value());
	}
} // namespace MEM

// clang-format off

#define HOOK_SIG(sig, fnHook, fnTrampoline) \
	static auto fn##fnHook = GAMEDATA::GetMemSig(sig); \
	SDK_ASSERT(fn##fnHook); \
	if (fn##fnHook) { \
		MEM::AddDetour(fn##fnHook, fnHook, fnTrampoline); \
	}

#define HOOK_VMT(instance, vfn, fnHook, fnTrampoline) \
	SDK_ASSERT(MEM::AddVMTInstanceHook(instance, offsetof_vtablefn(vfn), fnHook, fnTrampoline));

#define HOOK_VMT_OVERRIDE(instance, classname, vfn, fnHook, fnTrampoline, ...) \
	SDK_ASSERT(MEM::AddVMTInstanceHook( \
		instance, \
		TOOLS::GetVtableIndex(static_cast<FunctionTraits<decltype(&fnHook)>::ReturnType (classname::*)(__VA_ARGS__)>(&classname::vfn)), \
		fnHook, \
		fnTrampoline));

#define HOOK_VMTEX(sClassname, vfn, pModule, fnHook, fnTrampoline) \
	SDK_ASSERT(MEM::AddVMTHookEx(pModule.get(), sClassname, offsetof_vtablefn(vfn), fnHook, fnTrampoline));

#define GAMEDATA_VMT(gdOffsetKey, pModule, fnHook, fnTrampoline) \
	SDK_ASSERT(MEM::AddVMTHookEx(pModule.get(), gdOffsetKey, GAMEDATA::GetOffset(gdOffsetKey), fnHook, fnTrampoline, false));

#define DETOUR_VMT(gdOffsetKey, pModule, fnHook, fnTrampoline) \
	SDK_ASSERT(MEM::AddVMTHookEx(pModule.get(), gdOffsetKey, GAMEDATA::GetOffset(gdOffsetKey), fnHook, fnTrampoline, true));

#define STORE_TO_ADDRESS(address, ...) MEM::PatchAddress(address, __VA_ARGS__)

#define STORE_TO_ADDRESS_WITH_OFFSET(address, offset, ...) MEM::PatchAddress<offset>(address, __VA_ARGS__)

#define CALL_VIRTUAL(retType, idx, ...)    MEM::CallVirtual<retType>(idx, __VA_ARGS__)

// clang-format on
