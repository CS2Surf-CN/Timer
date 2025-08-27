#include "surf_global.h"
#include <utils/utils.h>

json GlobalAPIResponse::ToJson() const {
	return json {{"code", this->m_iCode}, {"message", this->m_sMessage}, {"data", this->m_Data}, {"timestamp", this->m_iTimestamp}};
}

void GlobalAPIResponse::FromJson(const json& j) {
	JSON_GETTER(j, code, m_iCode);
	JSON_GETTER(j, message, m_sMessage);
	JSON_GETTER(j, data, m_Data);
	JSON_GETTER(j, timestamp, m_iTimestamp);
}

namespace SURF::GLOBALAPI {
	namespace MAP {
		zoneinfo_t::zoneinfo_t()
			: ZoneData_t() {
			m_sMap = GlobalPlugin()->m_sMapName;
		}

		zoneinfo_t::zoneinfo_t(const ZoneData_t& other)
			: ZoneData_t(other) {
			m_sMap = GlobalPlugin()->m_sMapName;
		}

		json zoneinfo_t::ToJson() const {
			// clang-format off
			json j {
				{"map", m_sMap}, 
				{"track", m_iTrack}, 
				{"type", m_iType}, 
				{"value", m_iValue}, 
				{"mins_x", m_vecMins.x}, 
				{"mins_y", m_vecMins.y}, 
				{"mins_z", m_vecMins.z}, 
				{"maxs_x", m_vecMaxs.x}, 
				{"maxs_y", m_vecMaxs.y}, 
				{"maxs_z", m_vecMaxs.z},
				{"des_pos_x", m_vecDestination.x},
				{"des_pos_y", m_vecDestination.y},
				{"des_pos_z", m_vecDestination.z},
			};
			// clang-format on

			if (m_iDatabaseID != -1) {
				j["id"] = m_iDatabaseID;
			}

			if (m_angDestination.IsValid()) {
				j["des_ang_x"] = m_angDestination.x;
				j["des_ang_y"] = m_angDestination.y;
				j["des_ang_z"] = m_angDestination.z;
			}

			if (m_iFlag != -1) {
				j["flags"] = m_iFlag;
			}

			if (!m_sHookHammerid.empty()) {
				j["hammerid"] = m_sHookHammerid;
			}

			if (!m_sHookName.empty()) {
				j["hookname"] = m_sHookName;
			}

			if (m_iLimitSpeed != -1) {
				j["limitspeed"] = m_iLimitSpeed;
			}

			return j;
		}

		void zoneinfo_t::FromJson(const nlohmann::json& j) {
			JSON_GETTER(j, id, m_iDatabaseID);
			JSON_GETTER(j, map, m_sMap);
			JSON_GETTER(j, type, m_iType);
			JSON_GETTER(j, mins_x, m_vecMins.x);
			JSON_GETTER(j, mins_y, m_vecMins.y);
			JSON_GETTER(j, mins_z, m_vecMins.z);
			JSON_GETTER(j, maxs_x, m_vecMaxs.x);
			JSON_GETTER(j, maxs_y, m_vecMaxs.y);
			JSON_GETTER(j, maxs_z, m_vecMaxs.z);
			JSON_GETTER(j, des_pos_x, m_vecDestination.x);
			JSON_GETTER(j, des_pos_y, m_vecDestination.y);
			JSON_GETTER(j, des_pos_z, m_vecDestination.z);
			JSON_GETTER(j, des_ang_x, m_angDestination.x);
			JSON_GETTER(j, des_ang_y, m_angDestination.y);
			JSON_GETTER(j, des_ang_z, m_angDestination.z);
			JSON_GETTER(j, track, m_iTrack);
			JSON_GETTER(j, flags, m_iFlag);
			JSON_GETTER(j, value, m_iValue);
			JSON_GETTER(j, hammerid, m_sHookHammerid);
			JSON_GETTER(j, hookname, m_sHookName);
			JSON_GETTER(j, limitspeed, m_iLimitSpeed);
		}
	} // namespace MAP
} // namespace SURF::GLOBALAPI
