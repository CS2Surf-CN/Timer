#include "surf_zones.h"
#include <surf/api.h>
#include <core/concmdmanager.h>
#include <core/menu.h>
#include <utils/utils.h>
#include <fmt/format.h>

static void ZoneMenu_SelectType(CSurfPlayer* pPlayer) {
	auto hMenu = MENU::Create(pPlayer->GetController());
	if (!hMenu) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = hMenu.Data();
	pMenu->SetTitle("选择类型");

	for (int i = EZoneType::Zone_Start; i < EZoneType::ZONETYPES_SIZE; i++) {
		pMenu->AddItem(SURF::ZONE::GetZoneNameByType((EZoneType)i), MENU_HANDLER_L(pPlayer) {
			auto& pZoneService = pPlayer->m_pZoneService;
			pZoneService->m_ZoneEdit.m_iType = (EZoneType)event.nItem;
			pZoneService->m_ZoneEdit.m_iValue = SURF::ZonePlugin()->GetZoneCount(pZoneService->m_ZoneEdit.m_iTrack, (EZoneType)event.nItem);
			pZoneService->m_ZoneEdit.StartEditZone();

			event.hMenu.CloseAll();
		});
	}

	pMenu->SetExitback(true);
	pMenu->Display();
}

static void ZoneMenu_SelectTrack(CSurfPlayer* pPlayer) {
	auto hMenu = MENU::Create(pPlayer->GetController());
	if (!hMenu) {
		SDK_ASSERT(false);
		return;
	}

	pPlayer->m_pZoneService->m_ZoneEdit.Reset();

	auto pMenu = hMenu.Data();
	pMenu->SetTitle("选择赛道");

	for (TimerTrack_t i = EZoneTrack::Track_Main; i < EZoneTrack::TRACKS_SIZE; i++) {
		pMenu->AddItem(SURF::GetTrackName(i), MENU_HANDLER_L(pPlayer) {
			pPlayer->m_pZoneService->m_ZoneEdit.m_iTrack = (EZoneTrack)event.nItem;
			ZoneMenu_SelectType(pPlayer);
		});
	}

	pMenu->SetExitback(true);
	pMenu->Display();
}

static void ZoneMenu_Edit(CSurfPlayer* pPlayer, bool bDelete = false) {
	auto& hZones = SURF::ZonePlugin()->m_hZones;
	if (hZones.empty()) {
		pPlayer->m_pZoneService->Print("找不到任何区域.");
		return;
	}

	auto hMenu = MENU::Create(pPlayer->GetController());
	if (!hMenu) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = hMenu.Data();
	pMenu->SetTitle(bDelete ? "删除区域" : "编辑区域");

	std::vector<std::pair<CZoneHandle, ZoneCache_t>> vZones(hZones.begin(), hZones.end());
	std::sort(vZones.begin(), vZones.end(), [](const auto& a, const auto& b) {
		const ZoneCache_t& zoneA = a.second;
		const ZoneCache_t& zoneB = b.second;

		if (zoneA.m_iTrack != zoneB.m_iTrack) {
			return zoneA.m_iTrack < zoneB.m_iTrack;
		}
		if (zoneA.m_iType != zoneB.m_iType) {
			return zoneA.m_iType < zoneB.m_iType;
		}
		return zoneA.m_iValue < zoneB.m_iValue;
	});

	for (const auto& [_, zone] : vZones) {
		std::string sZone = fmt::format("{} - {} #{}", SURF::GetTrackName(zone.m_iTrack), SURF::ZONE::GetZoneNameByType(zone.m_iType), zone.m_iValue);
		pMenu->AddItem(sZone, MENU_HANDLER_L(sZone, zone, pPlayer, bDelete) {
			if (bDelete) {
				pPlayer->m_pZoneService->DeleteZone(zone);
			} else {
				pPlayer->m_pZoneService->ReEditZone(zone);
			}

			pPlayer->Print("你选择了: %s", sZone.c_str());
		});
	}

	pMenu->SetExitback(true);
	pMenu->Display();
}

static void ZoneMenu_DeleteAll(CSurfPlayer* pPlayer) {
	if (SURF::ZonePlugin()->m_hZones.empty()) {
		pPlayer->m_pZoneService->Print("找不到任何区域.");
		return;
	}

	pPlayer->m_pZoneService->DeleteAllZones();
}

CCMD_CALLBACK(Command_Zones) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return;
	}

	auto hMenu = MENU::Create(pController);
	if (!hMenu) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = hMenu.Data();
	pMenu->SetTitle("区域菜单");

	pMenu->AddItem("添加", MENU_HANDLER_L(pPlayer) { ZoneMenu_SelectTrack(pPlayer); });
	pMenu->AddItem("编辑", MENU_HANDLER_L(pPlayer) { ZoneMenu_Edit(pPlayer); });
	pMenu->AddItem("删除", MENU_HANDLER_L(pPlayer) { ZoneMenu_Edit(pPlayer, true); });
	pMenu->AddItem("删除所有", MENU_HANDLER_L(pPlayer) { ZoneMenu_DeleteAll(pPlayer); });
	pMenu->AddItem("刷新", MENU_HANDLER_L(pPlayer) { SURF::ZonePlugin()->RefreshZones(); });

	pMenu->Display();
}

CCMD_CALLBACK(Command_EditZone) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	ZoneMenu_Edit(player);
}

void CSurfZonePlugin::RegisterCommand() {
	CONCMD::RegConsoleCmd("sm_zones", Command_Zones);
	CONCMD::RegConsoleCmd("sm_editzone", Command_EditZone);
}
