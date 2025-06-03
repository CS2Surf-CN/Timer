#pragma once

#include <surf/surf_player.h>
#include <surf/zones/edit.h>

struct ZoneCache_t : ZoneData_t {
	ZoneCache_t() : ZoneData_t() {
		m_aBeams.fill(CEntityHandle());
	}

	ZoneCache_t(const ZoneData_t& other) : ZoneData_t(other) {
		m_aBeams.fill(CEntityHandle());
	}

	std::array<CHandle<CBeam>, 12> m_aBeams;
};

using CZoneHandle = CHandle<CBaseTrigger>;

template<>
struct std::hash<CZoneHandle> {
	size_t operator()(const CZoneHandle& handle) const {
		return std::hash<int>()(handle.ToInt());
	}
};

class CSurfZoneService : public CSurfPlayerService {
private:
	virtual void OnInit() override;
	virtual void OnReset() override;

public:
	using CSurfPlayerService::CSurfPlayerService;

public:
	void EditZone(CCSPlayerPawnBase* pawn, const CInButtonState& buttons);
	void ReEditZone(const ZoneData_t& zone);
	void DeleteZone(const ZoneData_t& zone);
	void DeleteAllZones();
	bool TeleportToZone(TimerTrack_t track, EZoneType type);

private:
	void ResetCustomDestination();

public:
	std::array<std::array<std::pair<Vector, QAngle>, EZoneType::ZONETYPES_SIZE>, EZoneTrack::TRACKS_SIZE> m_aCustomDestination;
	ZoneEditProperty m_ZoneEdit;
};

class CSurfZonePlugin : CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnActivateServer(CNetworkGameServerBase* pGameServer) override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawnBase* pawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;
	virtual bool ProcessSayCommand(CCSPlayerController* pController) override;
	virtual bool OnSayCommand(CCSPlayerController* pController, const std::vector<std::string>& vArgs) override;

public:
	std::optional<ZoneCache_t> FindZone(CBaseEntity* pEnt);
	std::optional<std::pair<CZoneHandle, ZoneCache_t>> FindZone(TimerTrack_t track, EZoneType type, i32 value);
	int GetZoneCount(TimerTrack_t track, EZoneType type);
	std::vector<ZoneCache_t> GetZones(TimerTrack_t track, EZoneType type);
	void ClearZones();
	void RefreshZones();
	void UpsertZone(const ZoneData_t& data, bool bUpload = true);
	void DeleteZone(const ZoneData_t& data, bool bUpload = true);
	void DeleteAllZones(bool bUpload = true);
	void CreateBeams(const Vector& vecMin, const Vector& vecMax, std::array<CHandle<CBeam>, 12>& out);
	CBaseEntity* CreateNormalZone(const Vector& vecMins, const Vector& vecMaxs);
	void KillZone(const std::pair<CZoneHandle, ZoneCache_t>& zone);

private:
	void RegisterCommand();

public:
	std::unordered_map<CZoneHandle, ZoneCache_t> m_hZones;
};

namespace SURF {
	extern CSurfZonePlugin* ZonePlugin();

	namespace ZONE {
		void CreatePoints2D(const Vector& vecMin, const Vector& vecMax, Vector out[4]);
		void CreatePoints3D(const Vector& vecMin, const Vector& vecMax, Vector out[8]);
		void FillBoxMinMax(Vector& vecMin, Vector& vecMax, bool resize = false);
		Vector GetCenter(const Vector& vecMin, const Vector& vecMax);
		std::string GetZoneNameByType(EZoneType type);

		namespace HOOK {
			bool OnStartTouch(CBaseEntity* pSelf, CBaseEntity* pOther);
			void OnStartTouchPost(CBaseEntity* pSelf, CBaseEntity* pOther);
			void OnTouchPost(CBaseEntity* pSelf, CBaseEntity* pOther);
			void OnEndTouchPost(CBaseEntity* pSelf, CBaseEntity* pOther);
		} // namespace HOOK
	} // namespace ZONE
} // namespace SURF
