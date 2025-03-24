#pragma once

#include <pch.h>
#include <surf/api.h>

struct ReplayFrame_t {
	QAngle ang;
	Vector pos;
	uint64 buttons;
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
	virtual void OnPlayerRunCmdPost(CCSPlayerPawn* pawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;

	virtual bool OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* player) override;
	virtual bool OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* player) override;

private:
	void DoPlayback(CCSPlayerPawn* botPawn, CCSBot* pBot);

public:
	std::unordered_map<uint32_t, std::vector<ReplayFrame_t>> m_umTrackReplays;
	std::unordered_map<uint32_t, std::vector<ReplayFrame_t>> m_umStageReplays;
	std::array<uint32_t, MAXPLAYERS> m_iCurrentTick;
};

namespace SURF {
	extern CSurfReplayPlugin* ReplayPlugin();
} // namespace SURF
