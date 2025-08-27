#pragma once

#include <surf/api.h>

struct cp_cache_t : timer_snapshot_t, timer_pausemovement_t {};

class CSurfCheckpointPlugin : CSurfForward, CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnClientPutInServer(ISource2GameClients* pClient, CPlayerSlot slot, char const* pszName, int type, uint64 xuid) override;
	virtual void OnClientDisconnect(ISource2GameClients* pClient, CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID) override;

public:
	void RegisterCommands();

public:
	static void OnPlayerSpawn(IGameEvent* pEvent, const char* szName, bool bDontBroadcast);
};

class CSurfCheckpointService : public CSurfPlayerService {
public:
	using CSurfPlayerService::CSurfPlayerService;

	virtual void OnInit() override;
	virtual void OnPlayerQuit() override;

public:
	void OpenCheckpointsMenu();

	std::optional<cp_cache_t> GetCheckpoint(const i32 idx) const;
	bool EnsureIndex(const i32 idx) const;
	void ClampIndex(i32& idx) const;
	void SaveCheckpoint();
	void LoadCheckpoint(const cp_cache_t& cache);
	void LoadCheckpoint(const i32 idx);
	void LoadPrev();
	void LoadNext();
	void DeleteCheckpoint(const i32 idx);
	void ResetCheckpoint();
	i32 GetLatestCheckpoint() const;

	void PlaySaveCheckpointSound() const;
	void PlayTeleCheckpointSound() const;

public:
	std::vector<cp_cache_t> m_vCheckpoints;
	i32 m_iCurrentCP {};
	i32 m_iLastCPLoaded = -1;
};

namespace SURF {
	extern CSurfCheckpointPlugin* CheckpointPlugin();
} // namespace SURF
