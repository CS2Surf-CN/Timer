#include "surf_zones.h"
#include <surf/misc/surf_misc.h>
#include <regex>

// regex copy from: https://github.com/CS2Surf/Timer/blob/main/src/ST-Map/Map.cs
std::vector<ZoneData_t>& CSurfZonePlugin::BuildMappingZones() {
	static std::vector<ZoneData_t> ret;
	ret.clear();

	for (const auto& hTrigger : SURF::MiscPlugin()->m_vTriggers) {
		auto pTrigger = hTrigger.Get();
		if (!pTrigger) {
			SDK_ASSERT(false);
			return ret;
		}

		auto pszClassName = pTrigger->GetClassname();
		if (V_strcmp(pszClassName, "trigger_multiple")) {
			continue;
		}

		auto sTargetName = pTrigger->m_pEntity->m_name.String();
		bool bMapStartZone = V_strstr(sTargetName, "map_start") || V_strstr(sTargetName, "stage1_start") || V_strstr(sTargetName, "s1_start");
		bool bMapEndZone = V_strstr(sTargetName, "map_end");
		bool bStage = std::regex_match(sTargetName, std::regex("^s([1-9][0-9]?|tage[1-9][0-9]?)_start$"));
		bool bCheckPoint = std::regex_match(sTargetName, std::regex("^map_c(p[1-9][0-9]?|heckpoint[1-9][0-9]?)$"));
		bool bBonusStartZone = std::regex_match(sTargetName, std::regex("^b([1-9][0-9]?|onus[1-9][0-9]?)_start$"));
		bool bBonusEndZone = std::regex_match(sTargetName, std::regex("^b([1-9][0-9]?|onus[1-9][0-9]?)_end$"));
		bool bMatchMappingZone = bMapStartZone || bMapEndZone || bStage || bCheckPoint || bBonusStartZone || bBonusEndZone;

		if (bMatchMappingZone) {
			ZoneData_t data;
			data.EnsureDestination();
			if (data.m_vecDestination.Length() == 0.0f) {
				data.m_vecDestination = pTrigger->GetAbsOrigin();
			}

			data.m_sHookName = sTargetName;
			data.m_sHookHammerid = pTrigger->m_sUniqueHammerID();

			if (bMapStartZone) {
				data.m_iTrack = EZoneTrack::Track_Main;
				data.m_iType = EZoneType::Zone_Start;
				data.m_iValue = GetHookZoneCount(data.m_iTrack, data.m_iType);
			} else if (bMapEndZone) {
				data.m_iTrack = EZoneTrack::Track_Main;
				data.m_iType = EZoneType::Zone_End;
				data.m_iValue = GetHookZoneCount(data.m_iTrack, data.m_iType);
			} else if (bStage) {
				data.m_iTrack = EZoneTrack::Track_Main;
				data.m_iType = EZoneType::Zone_Stage;

				std::smatch matches;
				std::string sToMatch(sTargetName);
				if (!std::regex_search(sToMatch, matches, std::regex("[0-9][0-9]?"))) {
					data.m_iValue = std::stoi(matches.str());
				}
			} else if (bCheckPoint) {
				data.m_iTrack = EZoneTrack::Track_Main;
				data.m_iType = EZoneType::Zone_Checkpoint;

				std::smatch matches;
				std::string sToMatch(sTargetName);
				if (!std::regex_search(sToMatch, matches, std::regex("[0-9][0-9]?"))) {
					data.m_iValue = std::stoi(matches.str());
				}
			} else if (bBonusStartZone) {
				std::smatch matches;
				std::string sToMatch(sTargetName);
				if (!std::regex_search(sToMatch, matches, std::regex("[0-9][0-9]?"))) {
					data.m_iTrack = static_cast<EZoneTrack>(std::stoi(matches.str()));
				}

				data.m_iType = EZoneType::Zone_Start;
				data.m_iValue = GetHookZoneCount(data.m_iTrack, data.m_iType);
			} else if (bBonusEndZone) {
				std::smatch matches;
				std::string sToMatch(sTargetName);
				if (!std::regex_search(sToMatch, matches, std::regex("[0-9][0-9]?"))) {
					data.m_iTrack = static_cast<EZoneTrack>(std::stoi(matches.str()));
				}

				data.m_iType = EZoneType::Zone_End;
				data.m_iValue = GetHookZoneCount(data.m_iTrack, data.m_iType);
			}

			PrecacheHookZone(data);
			ret.emplace_back(data);
		}
	}

	return ret;
}
