#include "memory.h"
#include "gameconfig.h"
#include <sdk/common.h>
#include <libmem/libmem_helper.h>
#include <core/forwards.h>
#include <core/interfaces.h>

#include <sdk/usercmd.h>
#include <sdk/entity/services.h>
#include <sdk/entity/ccsplayerpawn.h>
#include <igamesystem.h>
#include <utils/ctimer.h>
#include <utils/utils.h>

#include <sourcehook.h>
#include <ISmmPlugin.h>

PLUGIN_GLOBALVARS();

template<typename T>
struct ReturnType;

template<typename Ret, typename... Args>
struct ReturnType<Ret (*)(Args...)> {
	using type = Ret;
};

#define CALL_SIG(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetMemSig(sig); \
	SURF_ASSERT(fnSig); \
	using FunctionType = decltype(&fnCurrent); \
	using ReturnTypeValue = ReturnType<FunctionType>::type; \
	return MEM::SDKCall<ReturnTypeValue>(fnCurrent, __VA_ARGS__);

#define CALL_ADDRESS(sig, fnCurrent, ...) \
	static auto fnSig = GAMEDATA::GetAddress(sig); \
	SURF_ASSERT(fnSig); \
	using FunctionType = decltype(&fnCurrent); \
	using ReturnTypeValue = ReturnType<FunctionType>::type; \
	return MEM::SDKCall<ReturnTypeValue>(fnCurrent, __VA_ARGS__);

#define HOOK_SIG(sig, fnHook, fnTrampoline) \
	static auto fn##fnHook = GAMEDATA::GetMemSig(sig); \
	SURF_ASSERT(fn##fnHook); \
	if (fn##fnHook) { \
		libmem::HookFunc(fn##fnHook, fnHook, fnTrampoline); \
	}

#define HOOK_VMT(gdOffsetKey, pModule, fnHook, fnTrampoline) \
	SURF_ASSERT(MEM::VmtHookEx(GAMEDATA::GetOffset(gdOffsetKey), pModule.get(), gdOffsetKey, fnHook, fnTrampoline));

void MEM::CALL::SwitchTeam(CCSPlayerController* controller, int team) {
	CALL_SIG("CCSPlayerController_SwitchTeam", SwitchTeam, controller, team);
}

void MEM::CALL::SetPawn(CBasePlayerController* controller, CCSPlayerPawn* pawn, bool a3, bool a4, bool a5) {
	CALL_SIG("CBasePlayerController_SetPawn", SetPawn, controller, pawn, a3, a4, a5);
}

#pragma region hooks

static void Hook_OnMovementServicesRunCmds(CPlayer_MovementServices* pMovementServices, CUserCmd* pUserCmd) {
	CCSPlayerPawn* pawn = pMovementServices->GetPawn();
	if (!pawn) {
		MEM::SDKCall<void>(MEM::g_fnMovementServicesRunCmds_Trampoline, pMovementServices, pUserCmd);
		return;
	}

	CCSPlayerController* controller = pawn->GetController<CCSPlayerController>();
	if (!controller) {
		MEM::SDKCall<void>(MEM::g_fnMovementServicesRunCmds_Trampoline, pMovementServices, pUserCmd);
		return;
	}

	int32_t seed;
	int32_t weapon;
	int32_t cmdnum;
	int32_t tickcount;
	int32_t mouse[2] = {0, 0};

	float vec[3] = {0.0f, 0.0f, 0.0f};
	float angles[3] = {0.0f, 0.0f, 0.0f};

	CInButtonStatePB* buttons_state = nullptr;
	CBaseUserCmdPB* baseCmd = pUserCmd->mutable_base();
	if (baseCmd) {
		weapon = baseCmd->weaponselect();
		cmdnum = baseCmd->legacy_command_number();
		tickcount = baseCmd->client_tick();
		seed = baseCmd->random_seed();
		mouse[0] = baseCmd->mousedx();
		mouse[1] = baseCmd->mousedy();
		vec[0] = baseCmd->leftmove();
		vec[1] = baseCmd->forwardmove();
		vec[2] = baseCmd->upmove();
		angles[0] = baseCmd->viewangles().x();
		angles[1] = baseCmd->viewangles().y();
		angles[2] = baseCmd->viewangles().z();
		buttons_state = baseCmd->mutable_buttons_pb();
	}

	CInButtonState* button = (CInButtonState*)buttons_state;

	bool block = false;
	for (auto p = CSurfForward::m_pFirst; p; p = p->m_pNext) {
		if (!p->OnPlayerRunCmd(pawn, button, vec, angles, weapon, cmdnum, tickcount, seed, mouse)) {
			block = true;
		}
	}

	if (baseCmd) {
		baseCmd->set_weaponselect(weapon);
		baseCmd->set_legacy_command_number(cmdnum);
		baseCmd->set_client_tick(tickcount);
		baseCmd->set_random_seed(seed);
		baseCmd->set_mousedx(mouse[0]);
		baseCmd->set_mousedy(mouse[1]);
		baseCmd->set_leftmove(vec[0]);
		baseCmd->set_forwardmove(vec[1]);
		baseCmd->set_upmove(vec[2]);

		if (baseCmd->has_viewangles()) {
			CMsgQAngle* viewangles = baseCmd->mutable_viewangles();
			if (viewangles) {
				viewangles->set_x(angles[0]);
				viewangles->set_y(angles[1]);
				viewangles->set_z(angles[2]);
			}
		}

		if (baseCmd->has_buttons_pb()) {
			CInButtonStatePB* buttons_pb = baseCmd->mutable_buttons_pb();
			if (buttons_pb) {
				buttons_pb->set_buttonstate1(button->pressed);
				buttons_pb->set_buttonstate2(button->changed);
				buttons_pb->set_buttonstate3(button->scroll);
			}
		}
	}

	if (!block) {
		MEM::SDKCall<void>(MEM::g_fnMovementServicesRunCmds_Trampoline, pMovementServices, pUserCmd);
	}

	for (auto p = CSurfForward::m_pFirst; p; p = p->m_pNext) {
		p->OnPlayerRunCmdPost(pawn, button, vec, angles, weapon, cmdnum, tickcount, seed, mouse);
	}
}

SH_DECL_HOOK1_void(IGameSystem, ServerGamePostSimulate, SH_NOATTRIB, false, const EventServerGamePostSimulate_t*);

static void Hook_OnServerGamePostSimulate(const EventServerGamePostSimulate_t* a2) {
	UTIL::ProcessTimers();
}

SH_DECL_HOOK3_void(ISource2Server, GameFrame, SH_NOATTRIB, false, bool, bool, bool);

static void Hook_OnGameFrame(bool simulating, bool bFirstTick, bool bLastTick) {
	for (auto p = CSurfForward::m_pFirst; p; p = p->m_pNext) {
		p->OnGameFrame(META_IFACEPTR(ISource2Server), simulating, bFirstTick, bLastTick);
	}
}

#pragma endregion

#pragma region setup

static bool SetupDetours() {
	// clang-format off
	HOOK_SIG("CPlayer_MovementServices::RunCmds", Hook_OnMovementServicesRunCmds, MEM::g_fnMovementServicesRunCmds_Trampoline);
	// clang-format on

	return true;
}

static bool SetupVMTHooks() {
	// clang-format off
	// HOOK_VMT("CEntityDebugGameSystem::ServerGamePostSimulate", MEM::MODULE::server, Hook_OnServerGamePostSimulate, MEM::g_fnServerGamePostSimulate_Trampoline);

	// clang-format on
	return true;
}

static bool SetupSourceHooks() {
	SH_ADD_HOOK(ISource2Server, GameFrame, IFACE::pServer, SH_STATIC(Hook_OnGameFrame), false);

	// clang-format off

	SH_ADD_DVPHOOK(IGameSystem, 
		ServerGamePostSimulate, 
		MEM::MODULE::server->GetVirtualTableByName("CEntityDebugGameSystem").RCast<IGameSystem*>(), 
		SH_STATIC(Hook_OnServerGamePostSimulate), 
		true
	);

	// clang-format on
	return true;
}

void MEM::SetupHooks() {
	SURF_ASSERT(SetupDetours());
	SURF_ASSERT(SetupVMTHooks());
	SURF_ASSERT(SetupSourceHooks());
}

void MEM::MODULE::Setup() {
	engine = std::make_shared<libmodule::CModule>();
	engine->InitFromName(LIB::engine2, true);

	tier0 = std::make_shared<libmodule::CModule>();
	tier0->InitFromName(LIB::tier0, true);

	server = std::make_shared<libmodule::CModule>();
	server->InitFromMemory(libmem::GetModule(LIB::server).base);

	schemasystem = std::make_shared<libmodule::CModule>();
	schemasystem->InitFromName(LIB::schemasystem, true);

	steamnetworkingsockets = std::make_shared<libmodule::CModule>();
	steamnetworkingsockets->InitFromName(LIB::steamnetworkingsockets, true);
}

#pragma endregion