#include "playermanager.h"
#include <sdk/entity/ccsplayercontroller.h>
#include <utils/utils.h>

CPlayerManager g_PlayerManager;

CPlayerManager* GetPlayerManager() {
	return &g_PlayerManager;
}

CCSPlayerController* CPlayer::GetController() const {
	auto entIndex = this->GetEntityIndex();
	CBaseEntity* ent = static_cast<CBaseEntity*>(GameEntitySystem()->GetEntityInstance(entIndex));
	if (!ent) {
		return nullptr;
	}

	return ent->IsController() ? static_cast<CCSPlayerController*>(ent) : nullptr;
}

CBasePlayerPawn* CPlayer::GetCurrentPawn() const {
	auto controller = this->GetController();
	return controller ? controller->GetCurrentPawn() : nullptr;
}

CCSPlayerPawn* CPlayer::GetPlayerPawn() const {
	auto controller = this->GetController();
	return controller ? controller->GetPlayerPawn() : nullptr;
}

CCSObserverPawn* CPlayer::GetObserverPawn() const {
	auto controller = this->GetController();
	return controller ? reinterpret_cast<CCSObserverPawn*>(controller->GetObserverPawn()) : nullptr;
}

CServerSideClient* CPlayer::GetClient() const {
	return UTIL::GetClientBySlot(GetPlayerSlot());
}

bool CPlayer::IsAuthenticated() const {
	auto client = GetClient();
	return client && client->IsConnected() && !client->IsFakeClient() && IFACE::pEngine->IsClientFullyAuthenticated(GetPlayerSlot());
}

bool CPlayer::IsConnected() const {
	auto client = GetClient();
	return client ? client->IsConnected() : false;
}

bool CPlayer::IsInGame() const {
	auto client = GetClient();
	return client ? client->IsInGame() : false;
}

bool CPlayer::IsFakeClient() const {
	auto client = GetClient();
	return client ? client->IsFakeClient() : false;
}

bool CPlayer::IsSourceTV() const {
	auto client = GetClient();
	return client ? client->IsHLTV() : false;
}

bool CPlayer::IsAlive() const {
	auto pawn = GetPlayerPawn();
	return pawn ? pawn->IsAlive() : false;
}

bool CPlayer::IsValidClient() const {
	if (!UTIL::IsPlayerSlot(m_iSlot)) {
		return false;
	}

	CServerSideClient* client = GetClient();
	if (!client) {
		return false;
	}

	auto controller = UTIL::GetController(client->GetPlayerSlot());
	return controller && controller->IsController() && client->IsConnected() && client->IsInGame() && !client->IsHLTV();
}

const char* CPlayer::GetName() const {
	auto controller = GetController();
	return controller ? controller->GetPlayerName() : "<blank>";
}

const char* CPlayer::GetIpAddress() const {
	auto client = GetClient();
	return client ? client->GetNetChannel()->GetRemoteAddress().ToString(true) : nullptr;
}

CSteamID CPlayer::GetSteamId(bool validated) const {
	static const CSteamID invalidId = k_steamIDNil;
	bool authenticated = this->IsAuthenticated();
	if (validated && !authenticated) {
		return invalidId;
	}

	CServerSideClient* client = GetClient();
	if (client && authenticated) {
		return client->GetClientSteamID();
	}

	return unauthenticatedSteamID;
}

void CPlayer::Kick(const char* internalReason, ENetworkDisconnectionReason reason) {
	IFACE::pEngine->KickClient(this->GetPlayerSlot(), internalReason, reason);
}

CPlayer* CPlayerManager::ToPlayer(CServerSideClientBase* pClient) const {
	return this->ToPlayer(CPlayerSlot(pClient->GetPlayerSlot()));
}

CPlayer* CPlayerManager::ToPlayer(CPlayerPawnComponent* component) const {
	return this->ToPlayer(component->GetPawn());
}

CPlayer* CPlayerManager::ToPlayer(CBasePlayerController* controller) const {
	if (!controller) {
		return nullptr;
	}

	return this->ToPlayer(CPlayerSlot(controller->GetPlayerSlot()));
}

CPlayer* CPlayerManager::ToPlayer(CBasePlayerPawn* pawn) const {
	if (!pawn) {
		return nullptr;
	}

	CBasePlayerController* controller = pawn->GetController();
	if (!controller) {
		return nullptr;
	}

	return this->ToPlayer(controller);
}

CPlayer* CPlayerManager::ToPlayer(CPlayerSlot slot) const {
	if (UTIL::IsPlayerSlot(slot)) {
		return m_pPlayers.at(slot.Get()).get();
	}

	return nullptr;
}

CPlayer* CPlayerManager::ToPlayer(CEntityIndex entIndex) const {
	CBaseEntity* ent = static_cast<CBaseEntity*>(GameEntitySystem()->GetEntityInstance(entIndex));
	if (!ent) {
		return nullptr;
	}

	if (ent->IsPawn()) {
		return this->ToPlayer(static_cast<CBasePlayerPawn*>(ent));
	}

	if (ent->IsController()) {
		return this->ToPlayer(static_cast<CBasePlayerController*>(ent));
	}

	return nullptr;
}

CPlayer* CPlayerManager::ToPlayer(CPlayerUserId userID) const {
	for (int slot = 0; slot < UTIL::GetGlobals()->maxClients; slot++) {
		if (g_pEngineServer->GetPlayerUserId(CPlayerSlot(slot)) == userID.Get()) {
			return this->ToPlayer(CPlayerSlot(slot));
		}
	}

	return nullptr;
}

CPlayer* CPlayerManager::ToPlayer(CSteamID steamid, bool validate) const {
	auto steam64 = steamid.ConvertToUint64();
	for (auto& player : GetOnlinePlayers()) {
		if (player->GetSteamId64(validate) == steam64) {
			return player;
		}
	}

	return nullptr;
}

std::vector<CPlayer*> CPlayerManager::GetOnlinePlayers() const {
	std::vector<CPlayer*> players;
	for (auto& player : m_pPlayers) {
		if (UTIL::IsPlayerSlot(player->GetPlayerSlot())) {
			players.emplace_back(player.get());
		}
	}

	return players;
}

void CPlayerManager::OnClientDisconnect(ISource2GameClients* pClient, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName,
										uint64 xuid, const char* pszNetworkID) {
	auto player = this->ToPlayer(slot);
	if (player) {
		player->Reset();
	}
}

void CPlayerManager::OnClientActive(ISource2GameClients* pClient, CPlayerSlot slot, bool bLoadGame, const char* pszName, uint64 xuid) {
	int iSlot = slot.Get();
	CPlayer* player = m_pPlayers.at(iSlot).get();
	if (player) {
		player->Init(iSlot);
		player->SetUnauthenticatedSteamID(xuid);
	}
}
