#include "surf_zones.h"
#include <utils/utils.h>
#include <core/sdkhook.h>
#include <surf/global/surf_global.h>

CSurfZonePlugin g_SurfZonePlugin;

CSurfZonePlugin* SURF::ZonePlugin() {
	return &g_SurfZonePlugin;
}

void CSurfZonePlugin::OnPluginStart() {
	RegisterCommand();
}

void CSurfZonePlugin::OnPlayerRunCmdPost(CCSPlayerPawn* pawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pawn);
	if (!player) {
		return;
	}

	player->m_pZoneService->EditZone(pawn, buttons);
}

std::optional<ZoneCache_t> CSurfZonePlugin::FindZone(CBaseEntity* pEnt) {
	auto handle = pEnt->GetRefEHandle();
	if (m_hZones.contains(handle)) {
		return m_hZones.at(handle);
	}

	return std::nullopt;
}

int CSurfZonePlugin::GetZoneCount(ZoneTrack track, ZoneType type) {
	int count = 0;
	for (const auto& pair : m_hZones) {
		const auto& cache = pair.second;
		if (cache.m_iTrack == track && cache.m_iType == type) {
			count++;
		}
	}

	return count;
}

void CSurfZonePlugin::ClearZones() {
	for (const auto& pair : m_hZones) {
		CZoneHandle hZone = pair.first;
		auto pZone = hZone.Get();
		if (pZone) {
			pZone->Kill();
		}

		auto& vBeams = pair.second.m_aBeams;
		for (const auto& hBeam : vBeams) {
			auto pBeam = hBeam.Get();
			if (pBeam) {
				pBeam->Kill();
			}
		}
	}

	m_hZones.clear();
}

void CSurfZonePlugin::RefreshZones() {
	ClearZones();

	SURF::GLOBALAPI::MAP::PullZone(HTTPRES_CALLBACK_L() {
		GAPIRES_CHECK(res, r);

		auto vZones = r.m_Data.get<std::vector<json>>();
		for (const auto& j : vZones) {
			if (j.is_null() || j.empty()) {
				continue;
			}

			SURF::GLOBALAPI::MAP::zoneinfo_t info;
			info.FromJson(j);

			SURF::ZonePlugin()->AddZone(info, false);
		}
	});
}

void CSurfZonePlugin::AddZone(const ZoneData_t& data, bool bUpload) {
	auto pZone = this->CreateNormalZone(data.m_vecMins, data.m_vecMaxs);
	CZoneHandle hRefZone = pZone->GetRefEHandle();
	ZoneCache_t cache(data);
	SURF::ZonePlugin()->CreateBeams(cache.m_vecMins, cache.m_vecMaxs, cache.m_aBeams);
	SURF::ZonePlugin()->m_hZones[hRefZone] = cache;

	if (bUpload) {
		SURF::GLOBALAPI::MAP::zoneinfo_t info(data);
		SURF::GLOBALAPI::MAP::UpdateZone(
			info, HTTPRES_CALLBACK_L() {
				GAPIRES_CHECK(res, r);
				UTIL::CPrintChatAll("更新区域成功!");
			});
	}
}

void CSurfZoneService::EditZone(CCSPlayerPawnBase* pawn, const CInButtonState& buttons) {
	auto& iEditStep = m_ZoneEdit.m_iStep;
	if (m_ZoneEdit.m_bEnabled) {
		trace_t tr;
		UTIL::GetPlayerAiming(pawn, tr);
		Vector& aim = tr.m_vEndPos;

		if (buttons.Pressed(IN_USE)) {
			iEditStep = (ZoneEditStep)(iEditStep + 1);

			m_ZoneEdit.CreateEditZone(aim);
		}

		m_ZoneEdit.UpdateZone(aim);
	}
}

void CSurfZonePlugin::CreateBeams(const Vector& vecMin, const Vector& vecMax, std::array<CHandle<CBeam>, 12>& out) {
	Vector points[8];
	SURF::ZONE::CreatePoints3D(vecMin, vecMax, points);
	for (int i = 0; i < 12; i++) {
		CBeam* beam = (CBeam*)UTIL::CreateBeam(points[CZoneEditProperty::m_iZonePairs3D[i][0]], points[CZoneEditProperty::m_iZonePairs3D[i][1]]);
		out[i] = beam->GetRefEHandle();
	}
}

CBaseEntity* CSurfZonePlugin::CreateNormalZone(const Vector& vecMins, const Vector& vecMaxs) {
	Vector vecCenter = (vecMins + vecMaxs) / 2.0;
	Vector mins(vecMins), maxs(vecMaxs);
	SURF::ZONE::FillBoxMinMax(mins, maxs, true);
	auto pZone = MEM::CALL::CreateAABBTrigger(vecCenter, mins, maxs);
	if (!pZone) {
		SDK_ASSERT(false);
		return nullptr;
	}

	SDKHOOK::HookEntity<SDKHook_StartTouch>(pZone, SURF::ZONE::HOOK::OnStartTouch);
	SDKHOOK::HookEntity<SDKHook_StartTouchPost>(pZone, SURF::ZONE::HOOK::OnStartTouchPost);
	SDKHOOK::HookEntity<SDKHook_TouchPost>(pZone, SURF::ZONE::HOOK::OnTouchPost);
	SDKHOOK::HookEntity<SDKHook_EndTouchPost>(pZone, SURF::ZONE::HOOK::OnEndTouchPost);

	pZone->m_pEntity->m_name = GameEntitySystem()->AllocPooledString("surf_zone");

	return pZone;
}

void CSurfZoneService::Reset() {
	m_ZoneEdit.Init(this);
}
