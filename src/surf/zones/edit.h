#pragma once

#include <surf/datatypes.h>

class CSurfZoneService;

enum ZoneEditStep : u8 {
	EditStep_None,   // 0 - nothing
	EditStep_First,  // 1 - wait for E tap to setup first coord
	EditStep_Second, // 2 - wait for E tap to setup second coord
	EditStep_Third,  // 3 - wait for E tap to setup height
	EditStep_Final   // 4 - confirm
};

struct ZoneData_t {
	ZoneData_t() {
		Reset();
	}

	ZoneData_t(const ZoneData_t& other) = default;

	ZoneData_t& operator=(const ZoneData_t& other) = default;

	void Reset() {
		m_iTrack = (EZoneTrack)-1;
		m_iType = (EZoneType)-1;
		m_iValue = -1;
		m_iDatabaseID = -1;
		m_vecMins = Vector(0.0f, 0.0f, 0.0f);
		m_vecMaxs = Vector(0.0f, 0.0f, 0.0f);
		m_vecDestination = Vector(0.0f, 0.0f, 0.0f);
		m_angDestination = QAngle(0.0f, 0.0f, 0.0f);
		m_iFlag = -1;
		m_sHookHammerid = {};
		m_sHookName = {};
		m_iLimitSpeed = 350;
	}

	bool IsInsideBox(const Vector& vec) const {
		return (vec.x > m_vecMins.x && vec.y > m_vecMins.y && vec.z > m_vecMins.z) && (vec.x < m_vecMaxs.x && vec.y < m_vecMaxs.y && vec.z < m_vecMaxs.z);
	}

	Vector GetCenter() const {
		return (m_vecMins + m_vecMaxs) / 2.0f;
	}

	bool IsHookZone() const {
		return !m_sHookHammerid.empty();
	}

	void EnsureDestination();

	EZoneTrack m_iTrack;
	EZoneType m_iType;
	i32 m_iValue;
	i32 m_iDatabaseID;
	Vector m_vecMins;
	Vector m_vecMaxs;
	Vector m_vecDestination;
	QAngle m_angDestination;
	i32 m_iFlag;
	i32 m_iLimitSpeed;
	std::string m_sHookHammerid;
	std::string m_sHookName;
};

struct ZoneEditProperty : ZoneData_t {
	using ZoneData_t::ZoneData_t;
	using ZoneData_t::operator=;

	void Init(CSurfZoneService* outer);
	void Reset();

	void StartEditZone();
	void CreateEditZone(const Vector& playerAim);
	void UpdateZone(const Vector& playerAim);
	void CreateZone2D(const Vector points[4], std::vector<CHandle<CBeam>>& out);
	void CreateZone3D(const Vector points[8], std::vector<CHandle<CBeam>>& out);
	void UpdateZone2D(const std::vector<CHandle<CBeam>>& vBeams, const Vector& vecMin, const Vector& vecMax);
	void UpdateZone3D(const std::vector<CHandle<CBeam>>& vBeams, const Vector& vecMin, const Vector& vecMax);
	void ClearBeams();
	void EnsureSettings();

	CSurfZoneService* m_pOuter;
	bool m_bEnabled;
	ZoneEditStep m_iStep;
	std::vector<CHandle<CBeam>> m_vBeam;

	bool m_bAwaitValueInput;
	bool m_bAwaitVelocityInput;

	static constexpr const int m_iZonePairs2D[][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
	static constexpr const int m_iZonePairs3D[][2] = {{0, 2}, {2, 6}, {6, 4}, {4, 0}, {0, 1}, {3, 1}, {3, 2}, {3, 7}, {5, 1}, {5, 4}, {6, 7}, {7, 5}};
};
