#include "surf_zones.h"
#include <core/concmdmanager.h>
#include <core/menu.h>
#include <utils/utils.h>

static void OpenMenu_SelectZoneType(CBasePlayerController* pController) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto menu = MENU::Create(MENU_CALLBACK_L(player) {
		auto& pZoneService = player->m_pZoneService;
		pZoneService->m_ZoneEdit.m_iType = (ZoneType)iItem;
		pZoneService->m_ZoneEdit.m_iValue = SurfZonePlugin()->GetZoneCount(pZoneService->m_ZoneEdit.m_iTrack, (ZoneType)iItem);
		UTIL::PrintChat(pController, "SELECT: %s, value: %d\n", CSurfZonePlugin::GetZoneNameByType((ZoneType)iItem).c_str(),
						pZoneService->m_ZoneEdit.m_iValue);
		pZoneService->m_ZoneEdit.StartEditZone();

		hMenu.Free();
	});

	menu->SetTitle("选择类型");
	for (int i = ZoneType::Zone_Start; i < ZoneType::ZONETYPES_SIZE; i++) {
		menu->AddItem(CSurfZonePlugin::GetZoneNameByType((ZoneType)i));
	}
	menu->Display(player->GetPlayerPawn());
}

static void OpenMenu_SelectZoneTrack(CBasePlayerController* pController) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto menu = MENU::Create(MENU_CALLBACK_L(player) {
		hMenu.Free();

		player->m_pZoneService->m_ZoneEdit.m_iTrack = (ZoneTrack)iItem;
		OpenMenu_SelectZoneType(pController);
	});

	menu->SetTitle("选择赛道");
	for (int i = ZoneTrack::Track_Main; i < ZoneTrack::TRACKS_SIZE; i++) {
		menu->AddItem(CSurfZonePlugin::GetZoneNameByTrack((ZoneTrack)i));
	}
	menu->Display(player->GetPlayerPawn());
}

CCMD_CALLBACK(Command_Zones) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto menu = MENU::Create(MENU_CALLBACK_L() {
		switch (iItem) {
			case 0:
				hMenu.Free();
				OpenMenu_SelectZoneTrack(pController);
				return;
			case 1:
				UTIL::PrintChat(pController, "SELECT 2\n");
				break;
		}

		hMenu.Free();
	});
	menu->SetTitle("区域菜单");
	menu->AddItem("添加");
	menu->AddItem("编辑");
	menu->Display(player->GetPlayerPawn());
}

CCMD_CALLBACK(Command_EditZone) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	auto& pZoneService = player->m_pZoneService;
	pZoneService->m_ZoneEdit.StartEditZone();
}

CCMD_CALLBACK(Command_TPStart) {
	CSurfPlayer* player = SURF::GetPlayerManager()->ToPlayer(pController);
	if (!player) {
		return;
	}

	player->GetPlayerPawn()->Teleport(&SurfZonePlugin()->m_vecTestStartZone, nullptr, nullptr);
}

class CDynamicProp : public CBaseEntity {
public:
	DECLARE_SCHEMA_CLASS(CDynamicProp);

	SCHEMA_FIELD(bool, m_bUseAnimGraph);
};

void RegisterCommand() {
	CONCMD::RegConsoleCmd("sm_zones", Command_Zones);
	CONCMD::RegConsoleCmd("sm_editzone", Command_EditZone);
	CONCMD::RegConsoleCmd("sm_r", Command_TPStart);
	CONCMD::RegConsoleCmd(
		"sm_ktest", CCMD_CALLBACK_L() {
			CBaseEntity* ent = MEM::CALL::CreateEntityByName("point_worldtext");
			CCSPlayerPawn* player = pController->GetPlayerPawn();
			if (!player) {
				return;
			}

			CEntityKeyValues* ent_kv = new CEntityKeyValues();
			Color text_color(128, 255, 124, 255);
			ent_kv->SetColor("color", text_color);
			ent_kv->SetBool("enabled", true);
			ent_kv->SetString("message", "v she ni ma si le");
			ent_kv->SetString("font_name", "Arial Black");
			ent_kv->SetFloat("world_units_per_pixel", 0.25f);
			ent_kv->SetFloat("depth_render_offset", 0.125f);
			ent_kv->SetInt("justify_horizontal", 0);
			ent_kv->SetInt("justify_vertical", 1);
			ent_kv->SetFloat("font_size", 40.0f);
			// ent_kv->SetString("scales", "20.0 1.0 1.0");

			CBaseViewModel* viewmodel = player->m_pViewModelServices()->GetViewModel();
			// if (!viewmodel) {
			//	return;
			// }

			QAngle ang = viewmodel->GetAbsAngles();
			ang.z = 90.0f;
			ang.y -= 90.0f;

			Vector fwd, right, up;
			AngleVectors(ang, &fwd, &right, &up);
			VectorNormalize(fwd);

			Vector origin = player->GetAbsOrigin();
			origin = origin + fwd * (-32.0f) + up * -16.0f;
			ent_kv->SetQAngle("angles", ang);
			ent_kv->SetVector("origin", origin);
			ent->DispatchSpawn(ent_kv);

			ent->SetParent(viewmodel);
		});
}
