#include "cs2surf.h"
#include "version.h"

#include <sdk/entity/cgameresourceservice.h>

#include <core/interfaces.h>
#include <core/memory.h>
#include <core/gamedata.h>
#include <core/cvarmanager.h>
#include <core/logger.h>
#include <core/gamesystem.h>

#include <movement/movement.h>

#include <utils/utils.h>

#include <surf/api.h>

#include <vendor/MultiAddonManager/public/imultiaddonmanager.h>

IMultiAddonManager* g_pMultiAddonManager;

CSurfPlugin g_SurfPlugin;

PLUGIN_EXPOSE(CSurfPlugin, g_SurfPlugin);

CSurfPlugin* SurfPlugin() {
	return &g_SurfPlugin;
}

bool CSurfPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) {
	PLUGIN_SAVEVARS();

	GAMEDATA::Append("cs2surf-core.games.jsonc");

	IFACE::Setup(ismm, error, maxlen);

	LOG::Setup(0x00FFFF);

	MEM::MODULE::Setup();
	MEM::SetupHooks();
	MOVEMENT::SetupHooks();
	GS::Setup();

	FORWARD_POST(CCoreForward, OnPluginStart);

	return true;
}

void CSurfPlugin::AllPluginsLoaded() {
	g_pMultiAddonManager = (IMultiAddonManager*)g_SMAPI->MetaFactory(MULTIADDONMANAGER_INTERFACE, nullptr, nullptr);
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
	return "yep";
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
	return "1337";
}

const char* CSurfPlugin::GetLogTag() {
	return "LEET";
}

class CGameEntitySystem;

CGameEntitySystem* GameEntitySystem() {
	return IFACE::pGameResourceServiceServer->GetGameEntitySystem();
}
