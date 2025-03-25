#pragma once

#include <pch.h>
#include <surf/api.h>

struct ReplayFrame_t {
	QAngle ang;
	Vector pos;
	CPlayerButton buttons;
	uint32 flags;
	MoveType_t mt;
};

class CSurfReplayService : CSurfBaseService {
private:
	virtual void Reset() override;

public:
	using CSurfBaseService::CSurfBaseService;

	void StartRecord();
	void DoRecord(CCSPlayerPawn* pawn, const CInButtonState& buttons, const QAngle& viewAngles);
	void SaveRecord();

public:
	bool m_bEnabled;
	std::vector<ReplayFrame_t> m_vReplayFrames;
};

class CSurfReplayPlugin : CSurfForward, CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;

	virtual bool OnPlayerRunCmd(CCSPlayerPawn* pPawn, CInButtonState& buttons, float (&vec)[3], QAngle& viewAngles, int& weapon, int& cmdnum, int& tickcount, int& seed, int (&mouse)[2]) override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawn* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;

	virtual bool OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;
	virtual bool OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;

private:
	void DoPlayback(CCSPlayerPawn* pBotPawn, CInButtonState& buttons);

public:
	std::unordered_map<uint32, std::vector<ReplayFrame_t>> m_umTrackReplays;
	std::unordered_map<uint32, std::vector<ReplayFrame_t>> m_umStageReplays;
	std::array<uint32, MAXPLAYERS> m_iCurrentTick;
};

namespace SURF {
	extern CSurfReplayPlugin* ReplayPlugin();
} // namespace SURF
