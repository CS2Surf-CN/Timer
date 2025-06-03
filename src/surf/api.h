#pragma once

#include <surf/zones/surf_zones.h>

constexpr auto SURF_TICKRATE = ENGINE_FIXED_TICK_RATE;

constexpr auto SURF_WORKSHOP_ADDONS_ID = "3377250885";
constexpr auto SURF_WORKSHOP_ADDONS_SNDEVENT_FILE = "soundevents/surf.vsndevts";

constexpr auto SURF_SND_ERROR = "Buttons.snd8";
constexpr auto SURF_SND_SAVE_CP = "surf.savecp";
constexpr auto SURF_SND_TELE_CP = "Buttons.snd9";

constexpr auto SURF_SND_TIMER_START = "surf.timer_start";
constexpr auto SURF_SND_TIMER_FINISH_MAIN = "surf.finish_main";
constexpr auto SURF_SND_TIMER_FINISH_BONUS = "surf.finish_bonus";
constexpr auto SURF_SND_TIMER_LOSSPB = "tr.PuckFail";
constexpr auto SURF_SND_TIMER_RESERVED = "tr.ScoreRegular";
constexpr auto SURF_SND_TIMER_RESERVED2 = "Instructor.ImportantLessonStart";
constexpr auto SURF_SND_TIMER_RESERVED3 = "Vote.Passed";
constexpr auto SURF_SND_TIMER_RESERVED4 = "UI.DeathNotice";

constexpr auto SURF_MAX_TRACK = EZoneTrack::TRACKS_SIZE;
constexpr auto SURF_MAX_STAGE = 64; // surf_classics3 is crazy

constexpr auto SURF_REPLAY_FORMAT = "{CS2SURF-CN-REPLAY}";
constexpr auto SURF_REPLAY_VERSION = 1;
constexpr auto SURF_REPLAY_RESIZE_THRESHOLD = 2;

class CSurfForward : public CBaseForward<CSurfForward> {
public:
	virtual bool OnTimerStart(CSurfPlayer* pPlayer) {
		return true;
	}

	virtual bool OnTimerPause(CSurfPlayer* pPlayer) {
		return true;
	}

	virtual bool OnTimerResume(CSurfPlayer* pPlayer) {
		return true;
	}

	virtual bool OnTimerStop(CSurfPlayer* pPlayer) {
		return true;
	}

	virtual void OnTimerStopPost(CSurfPlayer* pPlayer) {}

	virtual bool OnTimerFinish(CSurfPlayer* pPlayer) {
		return true;
	}

	virtual void OnTimerFinishPost(CSurfPlayer* pPlayer) {}

	virtual bool OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) {
		return true;
	}

	virtual bool OnStayZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) {
		return true;
	}

	virtual bool OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) {
		return true;
	}
};

class CSurfGlobalAPIForward : public CBaseForward<CSurfGlobalAPIForward> {
public:
	virtual void OnGlobalInit() {}

	virtual void OnGlobalZoneHelperInit() {}

	virtual void OnGlobalMapValidated() {}
};

namespace SURF {
	std::string GetStyleName(TimerStyle_t style);
	std::string GetStyleShortName(TimerStyle_t style);
	std::string GetTrackName(TimerTrack_t track);
	void CPrintChat(CPlayer* pPlayer, const char* fmt, ...);
	void CPrintChatAll(const char* fmt, ...);
} // namespace SURF
