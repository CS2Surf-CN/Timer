#pragma once

#include <pch.h>
#include <surf/api.h>
#include <surf/surf_bot.h>
#include <utils/ctimer.h>

using ReplayArray_t = std::vector<replay_frame_data_t>;

struct replay_run_info_t {
	std::time_t timestamp;
	uint64 steamid;
	f64 time;
	TimerStyle_t style;
	TimerTrack_t track;
	TimerStage_t stage;
	size_t framelength;
	// ReplayArray_t frames; // Not POD, write it on somewhere else
};

static_assert(std::is_trivial_v<replay_run_info_t> && std::is_standard_layout_v<replay_run_info_t>);

struct replay_file_header_t {
	std::string format = SURF_REPLAY_FORMAT;
	u8 version = SURF_REPLAY_VERSION;
	std::string map;
	f32 tickrate;
	replay_run_info_t info;

	void ReadFromStream(std::ifstream& in);
	void WriteToStream(std::ofstream& out) const;
};

struct replay_extraframe_t {
	size_t iPreStart {};
	size_t iPreEnd {};
	bool bGrabEnd {};
	size_t iEndStart {};
};

enum ReplayStatus : i8 {
	Replay_Start = 0,
	Replay_Running,
	Replay_End,
	Replay_Idle
};

enum ReplayBotType : i8 {
	Replay_Central = 0,
	Replay_Looping, // these are the ones that loop styles, tracks, and (eventually) stages...
	Replay_Dynamic, // these are bots that spawn on !replay when the central bot is taken
};

struct replay_bot_info_t {
	TimerStyle_t iStyle {};
	TimerTrack_t iTrack = -1;
	TimerStage_t iStage = -1;
	ReplayStatus iStatus = Replay_Idle;
	ReplayBotType iType = Replay_Central;
	bool b2x = false;
	f32 fRealTime {};
	size_t iTick {};
	size_t iRealTick {};
	f32 fRestartDelay = 1.0f;
	CHandle<CCSPlayerController> hStarter;
	CTimerHandle hRestartTimer;
};

class CSurfReplayService : CSurfPlayerService {
private:
	virtual void OnInit() override;
	virtual void OnPlayerQuit() override;

public:
	using CSurfPlayerService::CSurfPlayerService;

	void Init();
	void OnEnterStart_Recording();
	void OnStart_Recording();
	void OnTimerFinishPost_SaveRecording();
	void FinishGrabbingPostFrames(bool bStage = false);

	void DoRecord(CCSPlayerPawnBase* pawn, const CPlayerButton& buttons, const QAngle& viewAngles);
	void SaveRecord(bool bStageReplay, ReplayArray_t* out = nullptr);
	void ClearFrames();

public:
	bool m_bEnabled {};
	ReplayArray_t m_vCurrentFrames;
	size_t m_iCurrentFrame {};
	replay_extraframe_t m_ExtraTrackFrame;
	replay_extraframe_t m_ExtraStageFrame;

	CTimerHandle m_hTrackPostFrameTimer;
	CTimerHandle m_hStagePostFrameTimer;
};

class CSurfBotReplayService : CSurfBotService {
public:
	using CSurfBotService::CSurfBotService;

	virtual void OnInit() override;
	virtual void OnPlayerQuit() override;

public:
	void Init();
	void DoPlayback(CCSPlayerPawnBase* pPawn, CInButtonState& buttons, QAngle& viewAngles);
	void StartReplay(const replay_bot_info_t* info = nullptr);
	void FinishReplay(const replay_bot_info_t* info = nullptr);

	bool IsReplayBot() const {
		return m_bReplayBot;
	}

	bool IsStageBot() const {
		return m_info.iStage != -1;
	}

	bool IsTrackBot() const {
		return m_info.iTrack != -1;
	}

public:
	bool m_bReplayBot {};
	replay_bot_info_t m_info {};
};

class CSurfReplayPlugin : CSurfForward, CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnActivateServer(CNetworkGameServerBase* pGameServer) override;
	virtual void OnClientPutInServer(ISource2GameClients* pClient, CPlayerSlot slot, char const* pszName, int type, uint64 xuid) override;
	virtual void OnEntitySpawned(CEntityInstance* pEntity, bool bMapStarted) override;

	virtual bool OnPlayerRunCmd(CCSPlayerPawnBase* pPawn, CInButtonState& buttons, float (&vec)[3], QAngle& viewAngles, int& weapon, int& cmdnum, int& tickcount, int& seed, int (&mouse)[2]) override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;

	virtual bool OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;
	virtual bool OnStayZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;
	virtual bool OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;
	virtual void OnTimerFinishPost(CSurfPlayer* pPlayer) override;

public:
	std::string BuildReplayPath(const i8 style, const TimerTrack_t track, const i8 stage, const std::string_view map);
	void AsyncWriteReplayFile(const replay_run_info_t& info, const ReplayArray_t& vFrames);
	bool ReadReplayFile(const std::string_view path, ReplayArray_t& out);
	CSurfBot* CreateBot(const char* name = nullptr) const;

private:
	void HookEvents();

public:
	ConVarRefAbstract* m_cvarTrackPreRunTime;
	ConVarRefAbstract* m_cvarTrackPostRunTime;
	ConVarRefAbstract* m_cvarStagePreRunTime;
	ConVarRefAbstract* m_cvarStagePostRunTime;
	ConVarRefAbstract* m_cvarPreRunAlways;

	std::array<ReplayArray_t, SURF_MAX_TRACK> m_aTrackReplays {};
	std::array<ReplayArray_t, SURF_MAX_STAGE> m_aStageReplays {};

private:
	CPlayerSlot m_iLatestBot = -1;
};

namespace SURF {
	extern CSurfReplayPlugin* ReplayPlugin();

	namespace REPLAY {
		namespace HOOK {
			bool OnBotTrigger(CBaseEntity* pSelf, CBaseEntity* pOther);
			bool OnBotUse(CBaseEntity* pSelf, EntityInputData_t* pInput);
		} // namespace HOOK
	} // namespace REPLAY
} // namespace SURF
