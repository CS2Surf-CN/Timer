#include "utils.h"
#include <fstream>
#include <steam/isteamgameserver.h>
//#include <core/interfaces.h>
#include <public/iserver.h>
#include <sdk/entity/ccsplayercontroller.h>
#include <sdk/entity/ccsplayerpawn.h>
#include <cs2surf.h>
#include <worldsize.h>
#include <sdk/entity/recipientfilters.h>
#include <public/networksystem/inetworkmessages.h>
#include <usermessages.pb.h>

#define FCVAR_FLAGS_TO_REMOVE (FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY | FCVAR_DEFENSIVE)

std::string UTIL::GetWorkingDirectory() {
	return PATH::Join(std::filesystem::current_path().string(), "..", "..", GAME_NAME, "addons", "cs2surf");
}

std::string UTIL::GetPublicIP() {
	static std::string ipv4;
	if (!ipv4.empty()) {
		return ipv4;
	}

	auto sAddr = SteamGameServer()->GetPublicIP();
	if (!sAddr.IsSet()) {
		return std::string();
	}

	uint32_t ip = sAddr.m_unIPv4;
	std::ostringstream oss;

	// clang-format off
	oss << ((ip >> 24) & 0xFF) << "." 
		<< ((ip >> 16) & 0xFF) << "." 
		<< ((ip >>  8) & 0xFF) << "." 
		<< ((ip >>  0) & 0xFF);
	// clang-format on

	ipv4 = oss.str();
	return ipv4;
}

json UTIL::LoadJsonc(std::string sFilePath) {
	std::ifstream file(sFilePath);
	if (!file.good()) {
		return json();
	}

	return json::parse(file, nullptr, true, true);
}

std::wstring UTIL::ToWideString(const char* pszCharStr) {
	if (!pszCharStr || !pszCharStr[0]) {
		return std::wstring();
	}

	auto size_needed = WIN_LINUX(MultiByteToWideChar(CP_UTF8, 0, pszCharStr, -1, NULL, 0), mbstowcs(NULL, pszCharStr, 0));
	if (size_needed > 0) {
		std::wstring wSayContent;
		wSayContent.resize(size_needed, L'\0');
#ifdef _WIN32
		MultiByteToWideChar(CP_UTF8, 0, pszCharStr, -1, &wSayContent[0], size_needed);
#else
		mbstowcs(&wSayContent[0], pszCharStr, size_needed);
#endif
		return wSayContent;
	}

	return std::wstring();
}

void UTIL::ReplaceString(std::string& str, const char* search, const char* replace) {
	if (!search || !search[0] || !replace) {
		return;
	}

	size_t start_pos = 0;
	const size_t search_len = strlen(search);
	const size_t replace_len = strlen(replace);

	while ((start_pos = str.find(search, start_pos)) != std::string::npos) {
		str.replace(start_pos, search_len, replace);
		start_pos += replace_len;
	}
}

CGlobalVars* UTIL::GetGlobals() {
	INetworkGameServer* server = g_pNetworkServerService->GetIGameServer();

	if (!server) {
		return nullptr;
	}

	return server->GetGlobals();
}

CGlobalVars* UTIL::GetServerGlobals() {
	return SurfPlugin()->simulatingPhysics ? &(SurfPlugin()->serverGlobals) : GetGlobals();
};

CBasePlayerController* UTIL::GetController(CBaseEntity* entity) {
	CCSPlayerController* controller = nullptr;
	if (!V_stricmp(entity->GetClassname(), "observer")) {
		CBasePlayerPawn* pawn = static_cast<CBasePlayerPawn*>(entity);
		if (!pawn->m_hController()->IsValid() || pawn->m_hController()->Get() == 0) {
			for (i32 i = 0; i <= GetServerGlobals()->maxClients; i++) {
				controller = (CCSPlayerController*)UTIL::GetController(CPlayerSlot(i));
				if (controller && controller->m_hObserverPawn() && controller->m_hObserverPawn().Get() == entity) {
					return controller;
				}
			}
			return nullptr;
		}
		return pawn->m_hController()->Get();
	}
	if (entity->IsPawn()) {
		CBasePlayerPawn* pawn = static_cast<CBasePlayerPawn*>(entity);
		if (!pawn->m_hController()->IsValid() || pawn->m_hController()->Get() == 0) {
			// Seems like the pawn lost its controller, we can try looping through the controllers to find this pawn instead.
			for (i32 i = 0; i <= GetServerGlobals()->maxClients; i++) {
				controller = (CCSPlayerController*)UTIL::GetController(CPlayerSlot(i));
				if (controller && controller->m_hPlayerPawn() && controller->m_hPlayerPawn().Get() == entity) {
					return controller;
				}
			}
			return nullptr;
		}
		return pawn->m_hController()->Get();
	} else if (entity->IsController()) {
		return static_cast<CBasePlayerController*>(entity);
	} else {
		return nullptr;
	}
}

CBasePlayerController* UTIL::GetController(CPlayerSlot slot) {
	if (!GameEntitySystem() || !IsPlayerSlot(slot)) {
		return nullptr;
	}
	CBaseEntity* ent = static_cast<CBaseEntity*>(GameEntitySystem()->GetEntityInstance(CEntityIndex(slot.Get() + 1)));
	if (!ent) {
		return nullptr;
	}
	return ent->IsController() ? static_cast<CBasePlayerController*>(ent) : nullptr;
}

bool UTIL::IsPlayerSlot(CPlayerSlot slot) {
	int iSlot = slot.Get();
	return iSlot >= 0 && iSlot < GetGlobals()->maxClients;
}

CUtlVector<CServerSideClient*>* UTIL::GetClientList() {
	if (!g_pNetworkServerService) {
		return nullptr;
	}

	static int offset = GAMEDATA::GetOffset("ClientOffset");
	return reinterpret_cast<CUtlVector<CServerSideClient*>*>(((uint8_t*)g_pNetworkServerService->GetIGameServer() + offset));
}

CServerSideClient* UTIL::GetClientBySlot(CPlayerSlot slot) {
	return (GetClientList() && GetController(slot)) ? GetClientList()->Element(slot.Get()) : nullptr;
}

CCSGameRules* UTIL::GetGameRules() {
	CCSGameRulesProxy* proxy = nullptr;
	proxy = (CCSGameRulesProxy*)FindEntityByClassname(proxy, "cs_gamerules");
	if (proxy) {
		return proxy->m_pGameRules();
	}
	return nullptr;
}

std::string UTIL::GetCurrentMap() {
	return SurfPlugin()->m_sCurrentMap;
}

bool UTIL::TraceLine(const Vector& vecStart, const Vector& vecEnd, CEntityInstance* ignore1, CGameTrace* tr, uint64 traceLayer, uint64 excludeLayer) {
	Ray_t ray;
	CTraceFilter filter;
	filter.SetPassEntity1(ignore1);
	filter.m_nInteractsWith = traceLayer;
	filter.m_nInteractsExclude = excludeLayer;

	return MEM::CALL::TraceShape(ray, vecStart, vecEnd, filter, tr);
}

void UTIL::GetPlayerAiming(CCSPlayerPawnBase* pPlayer, CGameTrace& ret) {
	Vector from = pPlayer->GetEyePosition();

	Vector forward;
	AngleVectors(pPlayer->GetEyeAngle(), &forward);
	Vector to = from + forward * MAX_COORD_FLOAT;

	TraceLine(from, to, pPlayer, &ret, MASK_SOLID, CONTENTS_TRIGGER | CONTENTS_PLAYER | CONTENTS_PLAYER_CLIP);
}

void UTIL::PlaySoundToClient(CPlayerSlot player, const char* sound, f32 volume) {
	if (!SurfPlugin()->IsAddonMounted()) {
		return;
	}

	CSingleRecipientFilter filter(player.Get());
	EmitSound_t soundParams;
	soundParams.m_pSoundName = sound;
	soundParams.m_flVolume = volume;
	MEM::CALL::EmitSound(filter, player.Get() + 1, soundParams);
}

CBaseEntity* UTIL::FindEntityByClassname(CEntityInstance* start, const char* name) {
	if (!GameEntitySystem()) {
		return nullptr;
	}
	EntityInstanceByClassIter_t iter(start, name);

	return static_cast<CBaseEntity*>(iter.Next());
}

CBaseEntity* UTIL::CreateBeam(const Vector& from, const Vector& to, Color color, float width, CBaseEntity* owner) {
	CBeam* beam = (CBeam*)MEM::CALL::CreateEntityByName("beam");
	if (!beam) {
		return nullptr;
	}

	beam->Teleport(&from, nullptr, nullptr);

	beam->m_clrRender(color);
	beam->m_fWidth(width);
	beam->m_vecEndPos(to);
	beam->m_fadeMinDist(-1.0f);

	if (owner != nullptr) {
		beam->m_hOwnerEntity(owner->GetRefEHandle());
	}

	beam->DispatchSpawn();

	return beam;
}

void UTIL::UnlockConVars() {
	if (!g_pCVar) {
		return;
	}

	ConVarData* pCvar = nullptr;
	uint16 iCvarIdx = 0;

	// Can't use FindFirst/Next here as it would skip cvars with certain flags, so just loop through the handles
	do {
		ConVarRef hCvarHandle(iCvarIdx++);
		pCvar = g_pCVar->GetConVarData(hCvarHandle);

		if (!pCvar || !(pCvar->IsFlagSet(FCVAR_FLAGS_TO_REMOVE))) {
			continue;
		}

		pCvar->RemoveFlags(FCVAR_FLAGS_TO_REMOVE);
	} while (pCvar);
}

void UTIL::UnlockConCommands() {
	if (!g_pCVar) {
		return;
	}

	ConCommandData* pConCommand = nullptr;
	ConCommandData* pInvalidCommand = g_pCVar->GetConCommandData(ConCommandRef());
	uint16 iConCommandIdx = 0;

	do {
		pConCommand = g_pCVar->GetConCommandData(iConCommandIdx++);
		if (!pConCommand || pConCommand == pInvalidCommand || !(pConCommand->GetFlags() & FCVAR_FLAGS_TO_REMOVE)) {
			continue;
		}

		pConCommand->RemoveFlags(FCVAR_FLAGS_TO_REMOVE);
	} while (pConCommand && pConCommand != pInvalidCommand);
}

void UTIL::SendConVarValue(CPlayerSlot slot, const char* conVar, const char* value) {
	INetworkMessageInternal* netmsg = g_pNetworkMessages->FindNetworkMessagePartial("SetConVar");
	auto msg = netmsg->AllocateMessage()->ToPB<CNETMsg_SetConVar>();
	CMsg_CVars_CVar* cvar = msg->mutable_convars()->add_cvars();
	cvar->set_name(conVar);
	cvar->set_value(value);
	CSingleRecipientFilter filter(slot.Get());
	IFACE::pGameEventSystem->PostEventAbstract(0, false, &filter, netmsg, msg, 0);
	delete msg;
}

void UTIL::SendConVarValue(CPlayerSlot slot, ConVarRefAbstract* conVar, const char* value) {
	INetworkMessageInternal* netmsg = g_pNetworkMessages->FindNetworkMessagePartial("SetConVar");
	auto msg = netmsg->AllocateMessage()->ToPB<CNETMsg_SetConVar>();
	CMsg_CVars_CVar* cvar = msg->mutable_convars()->add_cvars();
	cvar->set_name(conVar->GetName());
	cvar->set_value(value);
	CSingleRecipientFilter filter(slot.Get());
	IFACE::pGameEventSystem->PostEventAbstract(0, false, &filter, netmsg, msg, 0);
	delete msg;
}

void UTIL::SendMultipleConVarValues(CPlayerSlot slot, const char** cvars, const char** values, u32 size) {
	INetworkMessageInternal* netmsg = g_pNetworkMessages->FindNetworkMessagePartial("SetConVar");
	auto msg = netmsg->AllocateMessage()->ToPB<CNETMsg_SetConVar>();
	for (u32 i = 0; i < size; i++) {
		CMsg_CVars_CVar* cvar = msg->mutable_convars()->add_cvars();
		cvar->set_name(cvars[i]);
		cvar->set_value(values[i]);
	}
	CSingleRecipientFilter filter(slot.Get());
	IFACE::pGameEventSystem->PostEventAbstract(0, false, &filter, netmsg, msg, 0);
	delete msg;
}

void UTIL::SendMultipleConVarValues(CPlayerSlot slot, ConVarRefAbstract** conVar, const char** values, u32 size) {
	INetworkMessageInternal* netmsg = g_pNetworkMessages->FindNetworkMessagePartial("SetConVar");
	auto msg = netmsg->AllocateMessage()->ToPB<CNETMsg_SetConVar>();
	for (u32 i = 0; i < size; i++) {
		CMsg_CVars_CVar* cvar = msg->mutable_convars()->add_cvars();
		cvar->set_name(conVar[i]->GetName());
		cvar->set_value(values[i]);
	}
	CSingleRecipientFilter filter(slot.Get());
	IFACE::pGameEventSystem->PostEventAbstract(0, false, &filter, netmsg, msg, 0);
	delete msg;
}
