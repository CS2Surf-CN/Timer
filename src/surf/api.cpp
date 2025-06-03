#include "api.h"
#include <fmt/format.h>
#include <utils/print.h>

FORWARD_INIT(CSurfForward);
FORWARD_INIT(CSurfGlobalAPIForward);

// TODO: Style
std::string SURF::GetStyleName(TimerStyle_t style) {
	return "正常模式";
}

std::string SURF::GetStyleShortName(TimerStyle_t style) {
	return "NM";
}

std::string SURF::GetTrackName(TimerTrack_t track) {
	switch (track) {
		case EZoneTrack::Track_Main:
			return "主线";
		default:
			return fmt::format("奖励 {}", (u8)track);
	}
}

void SURF::CPrintChat(CPlayer* pPlayer, const char* fmt, ...) {
	auto pController = pPlayer->GetController();
	if (pController) {
		CUtlString buffer;
		va_list args;
		va_start(args, fmt);
		buffer.FormatV(fmt, args);

		UTIL::CPrintChat(pController, "{green}[Surf] %s", buffer.Get());
	}
}

void SURF::CPrintChatAll(const char* fmt, ...) {
	auto vOnlinePlayers = SURF::GetPlayerManager()->GetOnlinePlayers();
	for (const auto& pPlayer : vOnlinePlayers) {
		auto pController = pPlayer->GetController();
		if (pController) {
			CUtlString buffer;
			va_list args;
			va_start(args, fmt);
			buffer.FormatV(fmt, args);

			UTIL::CPrintChat(pController, "{green}[Surf] %s", buffer.Get());
		}
	}
}
