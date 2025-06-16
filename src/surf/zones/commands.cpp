#include "surf_zones.h"
#include <surf/api.h>
#include <core/concmdmanager.h>
#include <core/menu.h>
#include <utils/utils.h>
#include <fmt/format.h>
#include <surf/global/surf_global.h>

static void ZoneMenu_SelectType(CSurfPlayer* pPlayer) {
	auto wpMenu = MENU::Create(
		pPlayer->GetController(), MENU_CALLBACK_L(pPlayer) {
			if (action == EMenuAction::SelectItem) {
				auto& pZoneService = pPlayer->m_pZoneService;
				pZoneService->m_ZoneEdit.m_iType = (EZoneType)iItem;
				pZoneService->m_ZoneEdit.m_iValue = SURF::ZonePlugin()->GetZoneCount(pZoneService->m_ZoneEdit.m_iTrack, (EZoneType)iItem);
				pZoneService->m_ZoneEdit.StartEditZone();

				hMenu.Close();
			}
		});

	if (wpMenu.expired()) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = wpMenu.lock();
	pMenu->SetTitle("选择类型");
	for (int i = EZoneType::Zone_Start; i < EZoneType::ZONETYPES_SIZE; i++) {
		pMenu->AddItem(SURF::ZONE::GetZoneNameByType((EZoneType)i));
	}
	pMenu->SetExitback(true);
	pMenu->Display();
}

static void ZoneMenu_SelectTrack(CSurfPlayer* pPlayer) {
	auto wpMenu = MENU::Create(
		pPlayer->GetController(), MENU_CALLBACK_L(pPlayer) {
			if (action == EMenuAction::SelectItem) {
				pPlayer->m_pZoneService->m_ZoneEdit.m_iTrack = (EZoneTrack)iItem;
				ZoneMenu_SelectType(pPlayer);
			}
		});

	if (wpMenu.expired()) {
		SDK_ASSERT(false);
		return;
	}

	pPlayer->m_pZoneService->m_ZoneEdit.Reset();

	auto pMenu = wpMenu.lock();
	pMenu->SetTitle("选择赛道");
	for (TimerTrack_t i = EZoneTrack::Track_Main; i < EZoneTrack::TRACKS_SIZE; i++) {
		pMenu->AddItem(SURF::GetTrackName(i));
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

	auto wpMenu = MENU::Create(
		pPlayer->GetController(), MENU_CALLBACK_L(pPlayer, bDelete) {
			if (action != EMenuAction::SelectItem) {
				return;
			}

			auto& item = hMenu.Data()->GetItem(iItem);
			if (!item.second.has_value()) {
				pPlayer->PrintWarning("未知错误: %s", FILE_LINE_STRING);
				return;
			}

			auto hZone = std::any_cast<CZoneHandle>(item.second);
			if (!SURF::ZonePlugin()->m_hZones.contains(hZone)) {
				pPlayer->PrintWarning("未知错误: %s", FILE_LINE_STRING);
				return;
			}

			auto& zone = SURF::ZonePlugin()->m_hZones.at(hZone);
			if (bDelete) {
				pPlayer->m_pZoneService->DeleteZone(zone);
			} else {
				pPlayer->m_pZoneService->ReEditZone(zone);
			}
			std::string sZone = fmt::format("{} - {} #{}", SURF::GetTrackName(zone.m_iTrack), SURF::ZONE::GetZoneNameByType(zone.m_iType), zone.m_iValue);
			pPlayer->Print("你选择了: %s", sZone.c_str());
		});

	if (wpMenu.expired()) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = wpMenu.lock();
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

	for (const auto& [handle, zone] : vZones) {
		std::string sZone = fmt::format("{} - {} #{}", SURF::GetTrackName(zone.m_iTrack), SURF::ZONE::GetZoneNameByType(zone.m_iType), zone.m_iValue);
		pMenu->AddItem(sZone, handle);
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

	auto wpMenu = MENU::Create(
		pController, MENU_CALLBACK_L(pPlayer) {
			if (action == EMenuAction::SelectItem) {
				switch (iItem) {
					case 0:
						ZoneMenu_SelectTrack(pPlayer);
						return;
					case 1:
						ZoneMenu_Edit(pPlayer);
						break;
					case 2:
						ZoneMenu_Edit(pPlayer, true);
						break;
					case 3:
						ZoneMenu_DeleteAll(pPlayer);
						break;
					case 4:
						SURF::ZonePlugin()->RefreshZones();
						break;
				}
			}
		});

	if (wpMenu.expired()) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = wpMenu.lock();
	pMenu->SetTitle("区域菜单");
	pMenu->AddItem("添加");
	pMenu->AddItem("编辑");
	pMenu->AddItem("删除");
	pMenu->AddItem("删除所有");
	pMenu->AddItem("刷新");
	pMenu->Display();
}

CCMD_CALLBACK(Command_EditZone) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	ZoneMenu_Edit(player);
}

CCMD_CALLBACK(Command_BuildMappingZones) {
	CSurfPlayer* pPlayer = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!pPlayer) {
		return;
	}

	pPlayer->m_pZoneService->Print("预建地图区域中...");
	auto& vHookZones = SURF::ZonePlugin()->BuildMappingZones();
	pPlayer->m_pZoneService->Print("预建地图完成!");

	auto wpMenu = MENU::Create(
		pController, MENU_CALLBACK_L(pPlayer, vHookZones) {
			if (action == EMenuAction::SelectItem) {
				switch (iItem) {
					case 0: {
						int iUploaded = 0;
						for (const auto& data : vHookZones) {
							const int iZonesToUpload = vHookZones.size();
							SURF::GLOBALAPI::MAP::zoneinfo_t info(data);
							SURF::GLOBALAPI::MAP::UpdateZone(
								info, HTTPRES_CALLBACK_L(&data, iZonesToUpload, &iUploaded) {
									GAPIRES_CHECK(res, r, SURF::CPrintChatAll("{darkred}上传失败, 区域: %s.", data.m_sHookHammerid.c_str()));
									iUploaded++;

									if (iUploaded == iZonesToUpload) {
										SURF::CPrintChatAll("{grey}所有区域已上传成功!");
									}
								});
						}
						break;
					}
					case 1: {
						pPlayer->m_pZoneService->Print("已取消.");
						break;
					}
				}

				hMenu.Close();
			}
		});

	if (wpMenu.expired()) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = wpMenu.lock();
	pMenu->SetTitle("是否上传至后台?");
	pMenu->AddItem("是");
	pMenu->AddItem("否");
	pMenu->Display();
}

void CSurfZonePlugin::RegisterCommand() {
	CONCMD::RegAdminCmd("sm_zones", Command_Zones, AdminFlag::Generic);
	CONCMD::RegAdminCmd("sm_editzone", Command_EditZone, AdminFlag::Generic);
	CONCMD::RegAdminCmd("sm_buildmapping", Command_BuildMappingZones, AdminFlag::Generic);
}
