#include "surf_zones.h"
#include <utils/utils.h>
#include <core/sdkhook.h>
#include <core/menu.h>
#include <surf/global/surf_global.h>
#include <surf/misc/surf_misc.h>
#include <surf/api.h>

CSurfZonePlugin g_SurfZonePlugin;

CSurfZonePlugin* SURF::ZonePlugin() {
	return &g_SurfZonePlugin;
}

void CSurfZonePlugin::OnPluginStart() {
	RegisterCommand();
}

void CSurfZonePlugin::OnActivateServer(CNetworkGameServerBase* pGameServer) {
	RefreshZones();
}

void CSurfZonePlugin::OnMapEnd() {
	ClearZones();
}

void CSurfZonePlugin::OnEntitySpawned(CEntityInstance* pEntity, bool bMapStarted) {
	if (m_vPrecacheHookZones.empty()) {
		return;
	}

	auto pszClassname = pEntity->GetClassname();
	if (bMapStarted && V_strstr(pszClassname, "trigger_")) {
		auto pszHammerid = reinterpret_cast<CBaseEntity*>(pEntity)->m_sUniqueHammerID().Get();
		if (pszHammerid) {
			auto it = std::ranges::find_if(m_vPrecacheHookZones, [pszHammerid](const ZoneData_t& data) { return !V_strcmp(data.m_sHookHammerid.c_str(), pszHammerid); });
			if (it != m_vPrecacheHookZones.end()) {
				const auto& data = *it;
				CreateHookZone(static_cast<CBaseEntity*>(pEntity), data);
				m_vPrecacheHookZones.erase(it);
			}
		}
	}
}

void CSurfZonePlugin::OnPlayerRunCmdPost(CCSPlayerPawnBase* pawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pawn);
	if (!pPlayer) {
		return;
	}

	pPlayer->m_pZoneService->EditZone(pawn, buttons);
}

bool CSurfZonePlugin::ProcessSayCommand(CCSPlayerController* pController) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return false;
	}

	auto& pZoneService = pPlayer->m_pZoneService;
	if (pZoneService->m_ZoneEdit.m_bAwaitValueInput || pZoneService->m_ZoneEdit.m_bAwaitVelocityInput) {
		return true;
	}

	return false;
}

bool CSurfZonePlugin::OnSayCommand(CCSPlayerController* pController, const std::vector<std::string>& vArgs) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer || vArgs.empty()) {
		return true;
	}

	auto& pZoneService = pPlayer->m_pZoneService;
	if (pZoneService->m_ZoneEdit.m_bAwaitValueInput) {
		pZoneService->m_ZoneEdit.m_iValue = std::stoi(vArgs.at(0));
		pZoneService->m_ZoneEdit.EnsureSettings();

		pZoneService->m_ZoneEdit.m_bAwaitValueInput = false;
		return false;
	} else if (pZoneService->m_ZoneEdit.m_bAwaitVelocityInput) {
		pZoneService->m_ZoneEdit.m_iLimitSpeed = std::stof(vArgs.at(0));
		pZoneService->m_ZoneEdit.EnsureSettings();

		pZoneService->m_ZoneEdit.m_bAwaitVelocityInput = false;
		return false;
	}

	return true;
}

std::optional<ZoneCache_t> CSurfZonePlugin::FindZone(CBaseEntity* pEnt) {
	auto handle = pEnt->GetRefEHandle();
	if (m_hZones.contains(handle)) {
		return m_hZones.at(handle);
	}

	return std::nullopt;
}

std::optional<std::pair<CZoneHandle, ZoneCache_t>> CSurfZonePlugin::FindZone(TimerTrack_t track, EZoneType type, i32 value) {
	for (const auto& pair : m_hZones) {
		const auto& cache = pair.second;
		if (cache.m_iTrack == track && cache.m_iType == type && cache.m_iValue == value) {
			return pair;
		}
	}

	return std::nullopt;
}

int CSurfZonePlugin::GetZoneCount(TimerTrack_t track, EZoneType type) {
	int count = 0;
	for (const auto& [_, data] : m_hZones) {
		if (data.m_iTrack == track && data.m_iType == type) {
			count++;
		}
	}

	return count;
}

int CSurfZonePlugin::GetHookZoneCount(TimerTrack_t track, EZoneType type) {
	int count = GetZoneCount(track, type);
	if (!count) {
		for (const auto& data : m_vPrecacheHookZones) {
			if (data.m_iTrack == track && data.m_iType == type) {
				count++;
			}
		}
	}

	return count;
}

std::vector<ZoneCache_t> CSurfZonePlugin::GetZones(TimerTrack_t track, EZoneType type) {
	std::vector<ZoneCache_t> vZones;
	for (const auto& [_, data] : m_hZones) {
		if (data.m_iTrack == track && data.m_iType == type) {
			vZones.emplace_back(data);
		}
	}

	return vZones;
}

void CSurfZonePlugin::ClearZones() {
	for (const auto& cache : m_hZones) {
		KillZone(cache);
	}

	m_hZones.clear();
}

void CSurfZonePlugin::RefreshZones() {
	SURF::CPrintChatAll("{grey}开始刷新区域...");

	std::string sLastQueryMap = UTIL::GetCurrentMap();
	SURF::GLOBALAPI::MAP::PullZone(HTTPRES_CALLBACK_L(sLastQueryMap) {
		if (sLastQueryMap != UTIL::GetCurrentMap()) {
			return;
		}

		GAPIRES_CHECK(res, r, SURF::CPrintChatAll("{darkred}刷新区域失败."));

		SURF::ZonePlugin()->ClearZones();

		auto vZones = r.m_Data.get<std::vector<json>>();
		for (const auto& j : vZones) {
			if (j.is_null() || j.empty()) {
				continue;
			}

			SURF::GLOBALAPI::MAP::zoneinfo_t info;
			info.FromJson(j);

			SURF::ZonePlugin()->UpsertZone(info, false);
		}

		SURF::CPrintChatAll("{grey}刷新区域成功!");
	});
}

void CSurfZonePlugin::UpsertZone(const ZoneData_t& data, bool bUpload) {
	DeleteZone(data, false);

	if (!data.IsHookZone()) {
		if (!CreateNormalZone(data)) {
			SURF::CPrintChatAll("{darkred}创建普通区域失败!");
			return;
		}
	} else {
		PrecacheHookZone(data);
	}

	if (bUpload) {
		SURF::GLOBALAPI::MAP::zoneinfo_t info(data);
		SURF::GLOBALAPI::MAP::UpdateZone(info, HTTPRES_CALLBACK_L() {
			GAPIRES_CHECK(res, r, SURF::CPrintChatAll("{darkred}更新区域失败."));
			SURF::CPrintChatAll("{grey}更新区域成功!");
		});
	}
}

void CSurfZonePlugin::DeleteZone(const ZoneData_t& data, bool bUpload) {
	if (bUpload) {
		SURF::GLOBALAPI::MAP::zoneinfo_t info(data);
		SURF::GLOBALAPI::MAP::DeleteZone(info, HTTPRES_CALLBACK_L() {
			GAPIRES_CHECK(res, r, SURF::CPrintChatAll("{darkred}删除区域失败."));
			SURF::CPrintChatAll("{grey}删除区域成功!");
		});
	}

	auto res = FindZone(data.m_iTrack, data.m_iType, data.m_iValue);
	if (res) {
		const auto& zone = res.value();
		KillZone(zone);

		m_hZones.erase(zone.first);
	}
}

void CSurfZonePlugin::DeleteAllZones(bool bUpload) {
	if (bUpload) {
		SURF::GLOBALAPI::MAP::DeleteAllZones(HTTPRES_CALLBACK_L() {
			GAPIRES_CHECK(res, r, SURF::CPrintChatAll("{darkred}删除所有区域失败."));
			SURF::CPrintChatAll("{grey}删除所有区域成功!");
		});
	}

	ClearZones();
}

void CSurfZoneService::EditZone(CCSPlayerPawnBase* pawn, const CInButtonState& buttons) {
	if (m_ZoneEdit.m_bEnabled) {
		trace_t tr;
		UTIL::GetPlayerAiming(pawn, tr);
		Vector& aim = tr.m_vEndPos;

		if (buttons.Pressed(IN_USE)) {
			m_ZoneEdit.m_iStep += 1;

			m_ZoneEdit.CreateEditZone(aim);
		}

		m_ZoneEdit.UpdateZone(aim);
	}
}

void CSurfZoneService::ReEditZone(const ZoneData_t& zone) {
	m_ZoneEdit = zone;
	m_ZoneEdit.EnsureSettings();
}

void CSurfZoneService::DeleteZone(const ZoneData_t& zone) {
	auto pPlayer = GetPlayer();
	if (!pPlayer) {
		SDK_ASSERT(false);
		return;
	}

	auto hMenu = MENU::Create(pPlayer->GetController());
	if (!hMenu) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = hMenu.Data();
	pMenu->SetTitle("确认删除?");

	pMenu->AddItem("是", MENU_HANDLER_L(pPlayer, zone) {
		pPlayer->m_pZoneService->Print("已确认!");
		SURF::ZonePlugin()->DeleteZone(zone);
		event.hMenu.CloseAll();
	});

	pMenu->AddItem("否", MENU_HANDLER_L(pPlayer, zone) {
		pPlayer->m_pZoneService->Print("已取消.");
		event.hMenu.CloseAll();
	});

	pMenu->Display();
}

void CSurfZoneService::DeleteAllZones() {
	auto pPlayer = GetPlayer();
	if (!pPlayer) {
		SDK_ASSERT(false);
		return;
	}

	auto hMenu = MENU::Create(pPlayer->GetController());
	if (!hMenu) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = hMenu.Data();
	pMenu->SetTitle("确认删除所有区域?");

	pMenu->AddItem("是", MENU_HANDLER_L(pPlayer) {
		pPlayer->m_pZoneService->Print("已确认!");
		SURF::ZonePlugin()->DeleteAllZones();
		event.hMenu.CloseAll();
	});

	pMenu->AddItem("否", MENU_HANDLER_L(pPlayer) {
		pPlayer->m_pZoneService->Print("已取消.");
		event.hMenu.CloseAll();
	});

	pMenu->Display();
}

bool CSurfZoneService::TeleportToZone(TimerTrack_t track, EZoneType type) {
	const auto vZones = SURF::ZonePlugin()->GetZones(track, type);
	if (vZones.empty()) {
		return false;
	}

	auto pPawn = GetPlayer()->GetPlayerPawn();
	if (!pPawn) {
		return false;
	}

	pPawn->SetMoveType(MOVETYPE_WALK);

	const ZoneCache_t& zone = vZones.at(0);
	Vector vecTargetOrigin = zone.m_vecDestination;
	QAngle vecTargetAng = zone.m_angDestination;

	const auto& customDestination = m_aCustomDestination.at(track).at(type);
	if (customDestination.first.IsValid()) {
		if (zone.IsInsideBox(customDestination.first)) {
			vecTargetOrigin = customDestination.first;
		}
	}
	if (customDestination.second.IsValid()) {
		vecTargetAng = customDestination.second;
	}

	GetPlayer()->Teleport(&vecTargetOrigin, vecTargetAng.IsValid() ? &vecTargetAng : nullptr, &SURF::ZERO_VEC);

	return true;
}

void CSurfZoneService::ResetCustomDestination() {
	for (auto& arr : m_aCustomDestination) {
		for (auto& custom : arr) {
			custom.first.Invalidate();
			custom.second.Invalidate();
		}
	}
}

void CSurfZonePlugin::CreateBeams(const Vector& vecMin, const Vector& vecMax, std::array<CHandle<CBeam>, 12>& out) {
	Vector points[8];
	SURF::ZONE::CreatePoints3D(vecMin, vecMax, points);
	for (int i = 0; i < 12; i++) {
		CBeam* beam = (CBeam*)UTIL::CreateBeam(points[ZoneEditProperty::m_iZonePairs3D[i][0]], points[ZoneEditProperty::m_iZonePairs3D[i][1]]);
		out[i] = beam->GetRefEHandle();
	}
}

CBaseEntity* CSurfZonePlugin::CreateNormalZone(const ZoneData_t& data) {
	Vector vecCenter = SURF::ZONE::GetCenter(data.m_vecMins, data.m_vecMaxs);
	Vector mins(data.m_vecMins), maxs(data.m_vecMaxs);
	SURF::ZONE::FillBoxMinMax(mins, maxs, true);
	auto pZone = MEM::CALL::CreateAABBTrigger(vecCenter, mins, maxs);
	if (!pZone) {
		SDK_ASSERT(false);
		return nullptr;
	}

	SDKHOOK::HookEntity<SDKHook_StartTouch>(pZone, SURF::ZONE::HOOK::OnStartTouch);
	SDKHOOK::HookEntity<SDKHook_StartTouch>(pZone, SURF::ZONE::HOOK::OnStartTouchPost);
	SDKHOOK::HookEntity<SDKHook_Touch>(pZone, SURF::ZONE::HOOK::OnTouchPost);
	SDKHOOK::HookEntity<SDKHook_EndTouch>(pZone, SURF::ZONE::HOOK::OnEndTouchPost);

	pZone->SetName("surf_zone");

	ZoneCache_t cache(data);
	CreateBeams(cache.m_vecMins, cache.m_vecMaxs, cache.m_aBeams);
	m_hZones[pZone->GetRefEHandle()] = cache;

	return pZone;
}

void CSurfZonePlugin::CreateHookZone(CBaseEntity* pEnt, const ZoneData_t& data) {
	SDKHOOK::HookEntity<SDKHook_StartTouch>(pEnt, SURF::ZONE::HOOK::OnStartTouch);
	SDKHOOK::HookEntity<SDKHook_StartTouch>(pEnt, SURF::ZONE::HOOK::OnStartTouchPost);
	SDKHOOK::HookEntity<SDKHook_Touch>(pEnt, SURF::ZONE::HOOK::OnTouchPost);
	SDKHOOK::HookEntity<SDKHook_EndTouch>(pEnt, SURF::ZONE::HOOK::OnEndTouchPost);

	ZoneCache_t cache(data);

	if (data.m_iType == EZoneType::Zone_Mark) {
		CreateBeams(cache.m_vecMins, cache.m_vecMaxs, cache.m_aBeams);
	}

	m_hZones[pEnt->GetRefEHandle()] = cache;
}

void CSurfZonePlugin::PrecacheHookZone(const ZoneData_t& data) {
	for (const auto& hTrigger : SURF::MiscPlugin()->m_vTriggers) {
		auto pTrigger = hTrigger.Get();
		if (!pTrigger) {
			continue;
		}

		std::string sHammerid = pTrigger->m_sUniqueHammerID().Get();
		if (data.m_sHookHammerid == sHammerid) {
			CreateHookZone(pTrigger, data);
			return;
		}
	}

	m_vPrecacheHookZones.emplace_back(data);
}

void CSurfZonePlugin::KillZone(const std::pair<CZoneHandle, ZoneCache_t>& zone) {
	const auto& hZone = zone.first;
	const auto& data = zone.second;

	if (!data.IsHookZone()) {
		CBaseEntity* pZone = hZone.Get();
		if (pZone) {
			pZone->Kill();
		}
	}

	for (const auto& hBeam : data.m_aBeams) {
		auto pBeam = hBeam.Get();
		if (pBeam) {
			pBeam->Kill();
		}
	}
}

void CSurfZoneService::OnInit() {
	m_ZoneEdit.Init(this);
	ResetCustomDestination();
}

void CSurfZoneService::OnReset() {
	ResetCustomDestination();
}
