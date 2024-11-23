#include "adminmanager.h"

bool ADMIN::IsAdmin(uint64 xuid) {
	auto it = m_umAdmins.find(xuid);
	if (it != m_umAdmins.end()) {
		auto& info = it->second;
		return info.m_nFlag > AdminFlag::None;
	}
}

bool ADMIN::IsAdmin(CCSPlayerController* controller) {
	if (!controller) {
		return true;
	}

	if (!controller->IsController() || !controller->InGame() || controller->IsBot()) {
		return false;
	}

	return IsAdmin(controller->m_steamID());
}

CAdminInfo ADMIN::GetAdmin(uint64 xuid) {
	auto it = m_umAdmins.find(xuid);
	if (it != m_umAdmins.end()) {
		return it->second;
	}

	return CAdminInfo {};
}

CAdminInfo ADMIN::GetAdmin(CCSPlayerController* controller) {
	return GetAdmin(controller->m_steamID());
}

bool ADMIN::CheckAccess(uint64 xuid, AdminFlag flag) {
	auto it = m_umAdmins.find(xuid);
	if (it != m_umAdmins.end()) {
		auto& info = it->second;
		return (info.m_nFlag & flag) == flag || ((info.m_nFlag & AdminFlag::Root) == AdminFlag::Root);
	}

	return false;
}

bool ADMIN::CheckAccess(CCSPlayerController* controller, AdminFlag flag) {
	return CheckAccess(controller->m_steamID(), flag);
}

void ADMIN::AddAdmin(uint64 xuid, std::string nick, AdminFlag flag) {
	CAdminInfo admin {.m_iSteamID = xuid, .m_nFlag = flag};

	m_umAdmins[xuid] = admin;
}
