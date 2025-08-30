module;

#include <utils/utils.h>
#include <platform.h>
#include <tier1/convar.h>

module surf.core.concmdmanager;

import surf.core;

class CConCmdManager : CCoreForward {
	virtual bool OnClientCommand(ISource2GameClients* pClient, CPlayerSlot slot, const CCommand& args) override;
	virtual bool OnDispatchConCommand(ICvar* pCvar, ConCommandRef cmd, const CCommandContext& ctx, const CCommand& args) override;

public:
	struct CConCmdInfo {
		ConCmd_Callback callback;
		AdminFlag adminFlags = AdminFlag::None;
		std::string description;
		uint64 cmdFlags = FCVAR_NONE;
	};

	struct CSrvCmdInfo {
		SrvCmd_Callback callback;
		std::string description;
		uint64 cmdFlags = FCVAR_NONE;
	};

	std::unordered_map<std::wstring, std::vector<CConCmdInfo>> m_umConCmds;
	std::unordered_map<std::wstring, std::vector<CSrvCmdInfo>> m_umSrvCmds;
	std::unordered_map<std::wstring, std::vector<ConCmdListener_Callback>> m_umConCmdListeners;
} g_ConCmdManager;

static void RegCmd(const std::string& cmd, const std::string& description, uint64 cmdFlag) {
	ConCommandCreation_t command;
	command.m_pszName = cmd.c_str();
	command.m_pszHelpString = description.c_str();
	command.m_nFlags = cmdFlag;
	g_pCVar->RegisterConCommand(command, FCVAR_GAMEDLL);
}

void CONCMD::RegServerCmd(std::string cmd, SrvCmd_Callback cb, std::string description, uint64 cmdFlag) {
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

	if (!cmd.starts_with("sm_")) {
		cmd = "sm_" + cmd;
	}

	CConCmdManager::CSrvCmdInfo info;
	info.callback = cb;
	info.description = description;
	info.cmdFlags = cmdFlag;
	g_manager.m_umSrvCmds[UTIL::ToWideString(cmd.c_str())].emplace_back(info);

	RegCmd(cmd, description, cmdFlag);
}

void CONCMD::RegConsoleCmd(std::string cmd, ConCmd_Callback cb, std::string description, uint64 cmdFlag) {
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

	if (!cmd.starts_with("sm_")) {
		cmd = "sm_" + cmd;
	}

	CConCmdManager::CConCmdInfo info;
	info.callback = cb;
	info.description = description;
	info.adminFlags = AdminFlag::None;
	info.cmdFlags = cmdFlag;
	g_manager.m_umConCmds[UTIL::ToWideString(cmd.c_str())].emplace_back(info);

	RegCmd(cmd, description, cmdFlag | FCVAR_CLIENT_CAN_EXECUTE);
}

void CONCMD::RegAdminCmd(std::string cmd, ConCmd_Callback cb, AdminFlag adminFlags, std::string description, uint64 cmdFlag) {
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

	if (!cmd.starts_with("sm_")) {
		cmd = "sm_" + cmd;
	}

	CConCmdManager::CConCmdInfo info;
	info.callback = cb;
	info.description = description;
	info.adminFlags = adminFlags;
	info.cmdFlags = cmdFlag;
	g_manager.m_umConCmds[UTIL::ToWideString(cmd.c_str())].emplace_back(info);

	RegCmd(cmd, description, cmdFlag | FCVAR_CLIENT_CAN_EXECUTE);
}

void CONCMD::AddCommandListener(std::string cmd, ConCmdListener_Callback cb) {
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

	g_manager.m_umConCmdListeners[UTIL::ToWideString(cmd.c_str())].emplace_back(cb);
}

std::optional<ConCommandRef> CONCMD::Find(const char* name) {
	ConCommandRef cmdRef = g_pCVar->FindConCommand(name, true);
	if (!cmdRef.IsValidRef()) {
		return std::nullopt;
	}

	return cmdRef;
}

static void ParseSayArgs(const std::string& sRaw, std::vector<std::string>& vArgs) {
	std::string currentArg;
	bool inQuotes = false;

	size_t start = sRaw.find(' ');
	if (start != std::string::npos) {
		++start;
	} else {
		start = 0;
	}

	while (start < sRaw.size() && sRaw[start] == ' ') {
		++start;
	}

	size_t rawSize = sRaw.size();
	for (size_t i = start; i < rawSize; ++i) {
		char c = sRaw[i];

		if (c == '"') {
			inQuotes = !inQuotes;

			if (!inQuotes) {
				vArgs.emplace_back(std::move(currentArg));
				currentArg = "";
			}
		} else if (c == ' ' && !inQuotes) {
			if (!currentArg.empty()) {
				vArgs.emplace_back(std::move(currentArg));
				currentArg = "";
			}
		} else {
			currentArg += c;
		}
	}

	if (!currentArg.empty()) {
		vArgs.emplace_back(std::move(currentArg));
	}
}

static void HandleSrvCommand(const CCommand& pCommand, const std::wstring& wCommand) {
	auto it = g_manager.m_umSrvCmds.find(wCommand);
	if (it == g_manager.m_umSrvCmds.end()) {
		return;
	}

	std::vector<std::string> vArgs;
	for (int i = 1; i < pCommand.ArgC(); i++) {
		vArgs.emplace_back(pCommand.Arg(i));
	}

	auto& vCmdInfo = it->second;
	for (const auto& info : vCmdInfo) {
		info.callback(vArgs);
	}
}

static bool HandleConCommand(CCSPlayerController* pController, const CCommand& pCommand, const std::wstring& wCommand, bool sayCommand, bool spaceFound) {
	auto bRegistedCmd = g_manager.m_umConCmds.contains(wCommand);
	auto bListenedCmd = g_manager.m_umConCmdListeners.contains(wCommand);
	if (!bRegistedCmd && !bListenedCmd) {
		return true;
	}

	std::vector<std::string> vArgs;

	if (sayCommand) {
		if (spaceFound) {
			std::string sRawContent(pCommand.ArgS());
			ParseSayArgs(sRawContent, vArgs);
		}
	} else {
		for (int i = 1; i < pCommand.ArgC(); i++) {
			vArgs.emplace_back(pCommand.Arg(i));
		}
	}

	if (bRegistedCmd) {
		auto& vCmdInfo = g_manager.m_umConCmds.at(wCommand);
		for (const auto& info : vCmdInfo) {
			if (info.adminFlags != AdminFlag::None) {
				if (!ADMIN::CheckAccess(pController->m_steamID(), info.adminFlags)) {
					sayCommand ? UTIL::PrintChat(pController, "Access denied") : UTIL::PrintConsole(pController, "Access denied");
					continue;
				}
			}

			info.callback(pController, vArgs, wCommand);
		}
	}

	if (bListenedCmd) {
		auto& vListenCmds = g_manager.m_umConCmdListeners.at(wCommand);
		for (const auto& callback : vListenCmds) {
			if (!callback(pController, vArgs, wCommand)) {
				return false;
			}
		}
	}

	return true;
}

bool CONCMD::CConCmdManager::OnClientCommand(ISource2GameClients* pClient, CPlayerSlot slot, const CCommand& args) {
	auto pszCommand = args.Arg(0);
	if (!pszCommand || !pszCommand[0]) {
		return true;
	}

	auto pController = (CCSPlayerController*)UTIL::GetController(slot);
	if (!pController) {
		return true;
	}

	auto wCommand = UTIL::ToWideString(pszCommand);
	if (!g_manager.m_umConCmdListeners.contains(wCommand)) {
		return true;
	}

	std::vector<std::string> vArgs;
	for (int i = 1; i < args.ArgC(); i++) {
		vArgs.emplace_back(args.Arg(i));
	}

	auto& vListenCmds = g_manager.m_umConCmdListeners.at(wCommand);
	for (const auto& callback : vListenCmds) {
		if (!callback(pController, vArgs, wCommand)) {
			return false;
		}
	}

	return true;
}

bool CONCMD::CConCmdManager::OnDispatchConCommand(ICvar* pCvar, ConCommandRef cmd, const CCommandContext& ctx, const CCommand& args) {
	auto pszCommand = args.Arg(0);
	if (!pszCommand || !pszCommand[0]) {
		return true;
	}

	auto pController = (CCSPlayerController*)UTIL::GetController(ctx.GetPlayerSlot());
	if (!pController) {
		HandleSrvCommand(args, UTIL::ToWideString(pszCommand));
		return true;
	}

	if (!strcmp(pszCommand, "say") || !strcmp(pszCommand, "say_team")) {
		auto pszSayContent = args.Arg(1);
		if (!pszSayContent || !pszSayContent[0]) {
			return true;
		}

		for (auto p = CCoreForward::m_pFirst; p; p = p->m_pNext) {
			if (p->ProcessSayCommand(pController)) {
				std::string sRawContent(args.ArgS());
				std::vector<std::string> vArgs;
				ParseSayArgs(sRawContent, vArgs);
				FORWARD_PRE(CCoreForward, OnSayCommand, false, pController, vArgs);
			}
		}

		auto wSayContent = UTIL::ToWideString(pszSayContent);
		wchar_t commandSymbol = wSayContent[0];
		if (commandSymbol != L'!' && commandSymbol != 65281 && commandSymbol != L'.' && commandSymbol != 12290 && commandSymbol != L'/') {
			return true;
		}

		size_t spacePos = wSayContent.find(L' ');
		bool bSpaceFound = (spacePos != std::wstring::npos);
		std::wstring wCommand = bSpaceFound ? wSayContent.substr(1, spacePos - 1) : wSayContent.substr(1);
		if (wCommand.empty()) {
			return true;
		}

		if (bSpaceFound) {
			wCommand.append(1, L'\0');
		}

		wCommand = L"sm_" + wCommand;

		HandleConCommand(pController, args, wCommand, true, bSpaceFound);

		if (commandSymbol == L'/') {
			return false;
		}
	} else {
		return HandleConCommand(pController, args, UTIL::ToWideString(pszCommand), false, false);
	}

	return true;
}
