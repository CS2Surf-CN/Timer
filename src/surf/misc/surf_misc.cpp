#include "surf_misc.h"
#include <surf/api.h>
#include <core/interfaces.h>
#include <core/logger.h>
#include <core/cvarmanager.h>
#include <core/concmdmanager.h>
#include <utils/utils.h>

CSurfMiscPlugin g_SurfMisc;

CSurfMiscPlugin* SURF::MiscPlugin() {
	return &g_SurfMisc;
}

void CSurfMiscPlugin::OnPluginStart() {
	TweakCvars();
	HookEvents();
	RegisterCommands();
}

void CSurfMiscPlugin::TweakCvars() {
	static constexpr const char* tweakCvars[] = {"sv_infinite_ammo", "bot_freeze", "bot_stop", "bot_zombie", "sv_debug_overlays_broadcast"};
	// static constexpr const char* tweakCmds[] = {""};

	for (size_t i = 0; i < ARRAYSIZE(tweakCvars); i++) {
		auto cvar = CVAR::Find(tweakCvars[i]);
		if (cvar) {
			cvar->RemoveFlags(FCVAR_CHEAT);
		} else {
			LOG::Warning("Warning: %s is not found!\n", tweakCvars[i]);
		}
	}

	/*for (size_t i = 0; i < ARRAYSIZE(tweakCmds); i++) {
		auto pConCommand = CONCMD::Find(tweakCmds[i]);
		if (pConCommand) {
			pConCommand->RemoveFlags(FCVAR_CHEAT);
		} else {
			LOG::Warning("Warning: %s is not found!\n", tweakCvars[i]);
		}
	}*/
}

void CSurfMiscPlugin::OnActivateServer(CNetworkGameServerBase* pGameServer) {
	IFACE::pEngine->ServerCommand("exec cs2surf.cfg");

	// Restart round to ensure settings (e.g. mp_weapons_allow_map_placed) are applied
	IFACE::pEngine->ServerCommand("mp_restartgame 1");
}

void CSurfMiscPlugin::OnMapEnd() {
	m_vTriggers.clear();
	m_vTeleDestination.clear();
}

void CSurfMiscPlugin::OnWeaponDropPost(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon, const int& iDropType, const Vector* targetPos) {
	pWeapon->AcceptInput("kill");
}

void CSurfMiscPlugin::OnWeaponSwitchPost(CCSPlayer_WeaponServices* pService, CBasePlayerWeapon* pWeapon) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pService);
	if (!pPlayer) {
		return;
	}

	if (pPlayer->m_pMiscService->m_bHideWeapons) {
		pPlayer->m_pMiscService->HideWeapons();
	}
}

void CSurfMiscPlugin::OnEntitySpawned(CEntityInstance* pEntity, bool bMapStarted) {
	if (bMapStarted) {
		const char* sClassname = pEntity->GetClassname();
		if (V_strstr(sClassname, "trigger_")) {
			m_vTriggers.emplace_back(pEntity->GetRefEHandle());
		}
		if (!V_strcmp(sClassname, "info_teleport_destination") || !V_strcmp(sClassname, "info_target")) {
			m_vTeleDestination.emplace_back(pEntity->GetRefEHandle());
		}
	}
}

void CSurfMiscPlugin::OnClientDisconnect(ISource2GameClients* pClient, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) {
	auto pController = dynamic_cast<CCSPlayerController*>(UTIL::GetController(slot));
	if (!pController) {
		return;
	}

	// Immediately remove the player off the list. We don't need to keep them around.
	pController->m_LastTimePlayerWasDisconnectedForPawnsRemove().SetTime(0.01f);
	pController->SwitchTeam(0);
}

bool CSurfMiscPlugin::OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(ms);
	if (!player) {
		return true;
	}

	static constexpr float DUCK_SPEED_NORMAL = 8.0f;
	static constexpr float DUCK_SPEED_MINIMUM = 6.0234375f; // Equal to if you just ducked/unducked for the first time in a while;

	// https://github.com/KZGlobalTeam/gokz/blob/4ce210ac8ac59916d7a668869dd3a0906cef08f9/addons/sourcemod/scripting/gokz-mode-simplekz.sp#L823
	float flDuckSpeed = ms->m_flDuckSpeed();
	if (!ms->m_bDucking() && flDuckSpeed < DUCK_SPEED_NORMAL) {
		ms->m_flDuckSpeed(DUCK_SPEED_NORMAL);
	} else if (flDuckSpeed < DUCK_SPEED_MINIMUM) {
		ms->m_flDuckSpeed(DUCK_SPEED_MINIMUM);
	}

	return true;
}

bool CSurfMiscPlugin::OnPlayerMove(CCSPlayer_MovementServices* ms, CMoveData* mv) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(ms);
	if (!pPlayer) {
		return true;
	}

	// due to GetMaxSpeed() got inlined
	mv->m_flMaxSpeed = pPlayer->m_fCurrentMaxSpeed;

	return true;
}

bool CSurfMiscPlugin::OnTakeDamage(CCSPlayerPawnBase* pVictim, CTakeDamageInfo* info) {
	return false;
}

void CSurfMiscPlugin::OnResourcePrecache(IEntityResourceManifest* pResourceManifest) {
	pResourceManifest->AddResource(SURF_WORKSHOP_ADDONS_SNDEVENT_FILE);
}

void CSurfMiscService::HideLegs() {
	CCSPlayerPawn* pPawn = this->GetPlayer()->GetPlayerPawn();
	if (!pPawn) {
		return;
	}

	if (this->m_bHideLegs) {
		pPawn->m_clrRender(Color(255, 255, 255, 254));
		pPawn->m_fEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
		if (auto pWeaponService = pPawn->m_pWeaponServices(); pWeaponService) {
			auto pWeapons = pWeaponService->m_hMyWeapons();
			FOR_EACH_VEC(*pWeapons, i) {
				auto pWeapon = pWeapons->Element(i).Get();
				pWeapon->m_fEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);
			}
		}
	} else {
		pPawn->m_clrRender(Color(255, 255, 255, 255));
		pPawn->m_fEffects(0);
		if (auto pWeaponService = pPawn->m_pWeaponServices(); pWeaponService) {
			auto pWeapons = pWeaponService->m_hMyWeapons();
			FOR_EACH_VEC(*pWeapons, i) {
				auto pWeapon = pWeapons->Element(i).Get();
				pWeapon->m_fEffects(0);
			}
		}
	}
}

void CSurfMiscService::HideWeapons() {
	CCSPlayerPawn* pPawn = this->GetPlayer()->GetPlayerPawn();
	if (!pPawn) {
		return;
	}

	if (auto pWeaponService = pPawn->m_pWeaponServices(); pWeaponService) {
		if (m_bHideWeapons) {
			pWeaponService->m_hActiveWeapon(nullptr);
		} else {
			if (pWeaponService->m_hMyWeapons()->Count()) {
				pWeaponService->m_hActiveWeapon(pWeaponService->m_hMyWeapons()->Tail());
			}
		}
	}
}

void CSurfMiscService::OnPlayerQuit() {}
