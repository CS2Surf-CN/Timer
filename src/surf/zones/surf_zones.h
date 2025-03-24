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

class CZoneHandle : public CHandle<CBaseTrigger> {
	using CHandle::CHandle;
};

template<>
struct std::hash<CZoneHandle> {
	size_t operator()(const CZoneHandle& handle) const {
		return std::hash<uint32>()(handle.ToInt());
	}
};

class CSurfZoneService : public CSurfBaseService {
private:
	virtual void Reset() override;

public:
	using CSurfBaseService::CSurfBaseService;

public:
	void EditZone(CCSPlayerPawnBase* pawn, const CInButtonState& buttons);

public:
	CZoneEditProperty m_ZoneEdit;
};

class CSurfZonePlugin : CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawn* pawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;

public:
	std::optional<ZoneCache_t> FindZone(CBaseEntity* pEnt);
	int GetZoneCount(ZoneTrack track, ZoneType type);
	void ClearZones();
	void RefreshZones();
	void AddZone(const ZoneData_t& data, bool bUpload = true);
	void CreateBeams(const Vector& vecMin, const Vector& vecMax, std::array<CHandle<CBeam>, 12>& out);
	CBaseEntity* CreateNormalZone(const Vector& vecMins, const Vector& vecMaxs);

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
		std::string GetZoneNameByTrack(ZoneTrack track);
		std::string GetZoneNameByType(ZoneType type);

		namespace HOOK {
			bool OnStartTouch(CBaseEntity* pSelf, CBaseEntity* pOther);
			void OnStartTouchPost(CBaseEntity* pSelf, CBaseEntity* pOther);
			void OnTouchPost(CBaseEntity* pSelf, CBaseEntity* pOther);
			void OnEndTouchPost(CBaseEntity* pSelf, CBaseEntity* pOther);
		} // namespace HOOK
	} // namespace ZONE
} // namespace SURF
