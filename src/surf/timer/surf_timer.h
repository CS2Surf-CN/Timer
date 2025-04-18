#pragma once

#include <surf/api.h>

class CSurfTimerPlugin : CSurfForward, CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnPhysicsSimulatePost(CCSPlayerController* pController) override;
	virtual bool OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* player) override;
	virtual bool OnStayZone(const ZoneCache_t& zone, CSurfPlayer* player) override;
	virtual bool OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;

private:
	void RegisterCommand();
};

class CSurfTimerService : public CSurfPlayerService, public timer_snapshot_t {
public:
	using CSurfPlayerService::CSurfPlayerService;

	void DoTimerStart();
	bool DoTimerStop();
	void DoTimerFinish();
	void DoTimerPause();
	void DoTimerResume();

	void BuildSnapshot(timer_snapshot_t& buffer) const;
	void FromSnapshot(const timer_snapshot_t& snapshot);

	void PlayStartTimerSound();
	void PlayFinishTimerSound(ZoneTrack iTrack);

public:
	f64 m_fLastEndTime {};
	f64 m_fLastFalseEndTime {};
	f64 m_fLastStartSoundTime {};
	f64 m_fLastMissedTimeSoundTime {};
	bool m_bValidTime {};

	timer_pausemovement_t m_Pause;
};

namespace SURF {
	extern CSurfTimerPlugin* TimerPlugin();

	void FormatTime(f64 time, char* output, u32 length, bool precise = true);
	CUtlString FormatTime(f64 time, bool precise = true);
	void FormatDiffTime(f64 time, char* output, u32 length, bool precise = true);
	CUtlString FormatDiffTime(f64 time, bool precise = true);
} // namespace SURF
