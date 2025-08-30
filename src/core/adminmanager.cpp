module;

#include <unordered_map>
//#include <sdk/entity/ccsplayercontroller.h>

module surf.core.adminmanager;

std::unordered_map<uint64, CAdminInfo> g_umAdmins;

bool ADMIN::IsAdmin(uint64 xuid) {
	auto it = g_umAdmins.find(xuid);
	if (it != g_umAdmins.end()) {
		auto& info = it->second;
		return info.m_nFlag > AdminFlag::None;
	}

	return false;
}

bool ADMIN::IsAdmin(CCSPlayerController* pController) {
	if (!pController) {
		return true;
	}

	/*if (!pController->IsController() || !pController->InGame() || pController->IsBot()) {
		return false;
	}

	return IsAdmin(pController->m_steamID());*/
}

CAdminInfo ADMIN::GetAdmin(uint64 xuid) {
	auto it = g_umAdmins.find(xuid);
	if (it != g_umAdmins.end()) {
		return it->second;
	}

	return CAdminInfo {};
}

CAdminInfo ADMIN::GetAdmin(CCSPlayerController* pController) {
	return {};
	//return GetAdmin(pController->m_steamID());
}

bool ADMIN::CheckAccess(uint64 xuid, AdminFlag flag) {
	auto it = g_umAdmins.find(xuid);
	if (it != g_umAdmins.end()) {
		auto& info = it->second;
		return (info.m_nFlag & flag) == flag || ((info.m_nFlag & AdminFlag::Root) == AdminFlag::Root);
	}

	return false;
}

bool ADMIN::CheckAccess(CCSPlayerController* controller, AdminFlag flag) {
	return false;
	//return CheckAccess(controller->m_steamID(), flag);
}

void ADMIN::AddAdmin(uint64 xuid, AdminFlag flag) {
	CAdminInfo admin {.m_iSteamID = xuid, .m_nFlag = flag};

	g_umAdmins[xuid] = admin;
}
