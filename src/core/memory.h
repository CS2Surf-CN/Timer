#pragma once

#include <pch.h>

#include <sdk/common.h>
#include <libmem/libmem_helper.h>
#include <vendor/libmodule/module.h>
#include <sdk/datatypes.h>

class CBasePlayerController;
class CCSPlayerController;
class CCSPlayerPawn;
class CEntityInstance;
class CEntityKeyValues;
class CBaseTrigger;

class GameSessionConfiguration_t {};

#define HOOK_SIG(sig, fnHook, fnTrampoline) \
	static auto fn##fnHook = GAMEDATA::GetMemSig(sig); \
	SURF_ASSERT(fn##fnHook); \
	if (fn##fnHook) { \
		libmem::HookFunc(fn##fnHook, fnHook, fnTrampoline); \
	}

#define HOOK_VMT(gdOffsetKey, pModule, fnHook, fnTrampoline) \
	SURF_ASSERT(MEM::VmtHookEx(GAMEDATA::GetOffset(gdOffsetKey), pModule.get(), gdOffsetKey, fnHook, fnTrampoline));

#define DETOUR_VMT(gdOffsetKey, pModule, fnHook, fnTrampoline) \
	SURF_ASSERT(MEM::VmtHookEx(GAMEDATA::GetOffset(gdOffsetKey), pModule.get(), gdOffsetKey, fnHook, fnTrampoline, true));

namespace MEM {
	namespace CALL {
		void SwitchTeam(CCSPlayerController* controller, int team);
		void SetPawn(CBasePlayerController* controller, CCSPlayerPawn* pawn, bool a3, bool a4, bool a5);
		IGameEventListener2* GetLegacyGameEventListener(CPlayerSlot slot);
		bool TraceShape(const Ray_t& ray, const Vector& vecStart, const Vector& vecEnd, const CTraceFilter& filter, CGameTrace* tr);
		void TracePlayerBBox(const Vector& start, const Vector& end, const bbox_t& bounds, CTraceFilter* filter, trace_t& pm);
		void InitPlayerMovementTraceFilter(CTraceFilterPlayerMovementCS& pFilter, CEntityInstance* pHandleEntity, uint64 interactWith,
										   int collisionGroup);
		void SnapViewAngles(CBasePlayerPawn* pawn, const QAngle& angle);
		void CEntityInstance_AcceptInput(CEntityInstance* pEnt, const char* pInputName, CEntityInstance* pActivator, CEntityInstance* pCaller,
										 variant_t* value, int nOutputID);
		CBaseEntity* CreateEntityByName(const char* pszName);
		void DispatchSpawn(CBaseEntity* pEnt, CEntityKeyValues* pInitKeyValue);
		CBaseTrigger* CreateAABBTrigger(const Vector& center, const Vector& mins, const Vector& maxs);
		void SetParent(CBaseEntity* pEnt, CBaseEntity* pParent);
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
		inline void* g_fnCreateGameEvent;
		inline void* g_fnFireGameEvent;
		inline void* g_fnWeaponDrop;
		inline void* g_fnTakeDamage;
	} // namespace TRAMPOLINE

	void SetupHooks();

	template<typename T, typename... Args>
	T SDKCall(void* pAddress, Args... args) {
		auto pFn = reinterpret_cast<T (*)(Args...)>(pAddress);
		SURF_ASSERT((uintptr_t)pFn);
		return pFn(args...);
	}

	template<typename T, typename... Args>
	auto GetFn(void* pAddress, Args... args) {
		return reinterpret_cast<T (*)(Args...)>(pAddress);
	}

	template<typename T>
	inline bool VmtHookEx(uint32_t uIndex, libmodule::CModule* pModule, const char* className, T pFunc, void*& pOriginFunc,
						  bool allInstance = false) {
		if (!pModule) {
			return false;
		}

		auto extractClassName = [](std::string input) {
			size_t pos = input.find("::");
			if (pos != std::string::npos) {
				return input.substr(0, pos);
			}
			return input;
		};

		auto sClassName = extractClassName(className);

		void* vtable = pModule->GetVirtualTableByName(sClassName);
		if (!vtable) {
			return false;
		}

		libmem::Vmt vmt((libmem::Address*)vtable);
		auto from = vmt.GetOriginal(uIndex);

		if (allInstance) {
			libmem::HookFunc((void*)from, pFunc, pOriginFunc);
		} else {
			vmt.Hook(uIndex, (libmem::Address)pFunc);
			pOriginFunc = (void*)from;
		}

		return true;
	}
} // namespace MEM
