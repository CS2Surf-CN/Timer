#include "menu.h"
#include <core/memory.h>
#include <core/concmdmanager.h>
#include <utils/utils.h>
#include <sdk/entity/cbaseentity.h>

#include <fmt/format.h>

constexpr float g_fMenuOffsetX = -11.6f;
constexpr float g_fMenuOffsetY = -6.1f;

Vector CWorldTextMenu::GetAimPoint(const Vector& eyePosition, const QAngle& eyeAngles, float distanceToTarget) {
	double pitch = eyeAngles.x * (M_PI / 180.0);
	double yaw = eyeAngles.y * (M_PI / 180.0);

	double targetX = eyePosition.x + distanceToTarget * std::cos(pitch) * std::cos(yaw);
	double targetY = eyePosition.y + distanceToTarget * std::cos(pitch) * std::sin(yaw);
	double targetZ = eyePosition.z - distanceToTarget * std::sin(pitch);

	return Vector(targetX, targetY, targetZ);
}

CWorldTextMenu::CWorldTextMenu(MenuHandler pFnHandler, std::string sTitle) : CBaseMenu(pFnHandler, sTitle) {
	CPointWorldText* pMenuEntity = (CPointWorldText*)MEM::CALL::CreateEntityByName("point_worldtext");
	if (!pMenuEntity) {
		SURF_ASSERT(false);
		return;
	}

	CEntityKeyValues* pMenuKV = new CEntityKeyValues();
	if (!pMenuKV) {
		pMenuEntity->Kill();
		SURF_ASSERT(false);
		return;
	}

	int fontSize = 40;
	pMenuKV->SetBool("enabled", false);
	pMenuKV->SetFloat("world_units_per_pixel", (0.25 / 1000) * fontSize);
	pMenuKV->SetFloat("depth_render_offset", 0.125);
	pMenuKV->SetInt("justify_horizontal", 0); // 0代表左对齐
	pMenuKV->SetInt("justify_vertical", 1);   // 1代表垂直居中
	pMenuKV->SetInt("reorient_mode", 0);      // don't change
	pMenuKV->SetInt("fullbright", 1);
	pMenuKV->SetFloat("font_size", fontSize);
	pMenuKV->SetString("font_name", "Consolas");
	pMenuKV->SetColor("color", Color(255, 100, 255, 255));

	pMenuEntity->DispatchSpawn(pMenuKV);
	this->m_hWorldText.Set(pMenuEntity);

	// background
	CPointWorldText* pMenuBackground = (CPointWorldText*)MEM::CALL::CreateEntityByName("point_worldtext");
	if (!pMenuBackground) {
		return;
	}

	auto pBackgroundKV = new CEntityKeyValues();
	if (!pBackgroundKV) {
		pMenuBackground->Kill();
		return;
	}

	// background
	int bgFontSize = 80;
	pBackgroundKV->SetColor("color", Color(50, 50, 50, 100));
	pBackgroundKV->SetBool("enabled", false);
	pBackgroundKV->SetFloat("world_units_per_pixel", (0.25 / 300) * bgFontSize);
	pBackgroundKV->SetFloat("depth_render_offset", 0.125);
	pBackgroundKV->SetInt("justify_horizontal", 0); // 0代表左对齐
	pBackgroundKV->SetInt("justify_vertical", 1);   // 1代表垂直居中
	pBackgroundKV->SetInt("reorient_mode", 0);      // don't change
	pBackgroundKV->SetInt("fullbright", 1);
	pBackgroundKV->SetFloat("font_size", bgFontSize);

	pMenuBackground->DispatchSpawn(pBackgroundKV);
	this->m_hBackground.Set(pMenuBackground);

	pMenuBackground->SetText("█");
}

CWorldTextMenu::~CWorldTextMenu() {
	CBaseMenu::~CBaseMenu();

	auto pWorldText = m_hWorldText.Get();
	if (pWorldText) {
		pWorldText->Kill();
	}

	auto pBackground = m_hBackground.Get();
	if (pBackground) {
		pBackground->Kill();
	}
}

void CWorldTextMenu::Display(CCSPlayerPawnBase* pPawn, int iPageIndex) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pPawn);
	if (!pMenuPlayer) {
		SURF_ASSERT(false);
		return;
	}

	pMenuPlayer->m_pCurrentMenu.m_pMenu = this;
	pMenuPlayer->m_iCurrentPage = iPageIndex;

	CPointWorldText* pMenuEntity = this->m_hWorldText.Get();
	if (!pMenuEntity) {
		SURF_ASSERT(false);
		return;
	}

	CBaseViewModel* pViewModel = pPawn->m_pViewModelServices()->GetViewModel(1);
	if (!pViewModel) {
		pViewModel = (CBaseViewModel*)MEM::CALL::CreateEntityByName("csgo_viewmodel");
		pPawn->m_pViewModelServices()->SetViewModel(1, pViewModel);
	}

	pMenuEntity->SetParent(pViewModel);
	pMenuEntity->m_hOwnerEntity(pViewModel->GetRefEHandle());

	auto formatItem = [](int iItemIndex, const std::string& sItem) { return !sItem.empty() ? fmt::format("{}.{}", iItemIndex, sItem) : ""; };
	bool bDrawPreview = (iPageIndex != 0);
	bool bDrawNext = ((this->m_vItems.size() > 0) && (iPageIndex < (this->m_vItems.size() - 1)));

	// clang-format off
	std::string sMenuText = fmt::format("{}\n\n"
										"{}\n"
										"{}\n"
										"{}\n"
										"{}\n"
										"{}\n"
										"{}\n\n"
										"{}\n"
										"{}\n"
										"9.退出",
										this->m_sTitle, 
										formatItem(1, GetItem(iPageIndex, 0)), 
										formatItem(2, GetItem(iPageIndex, 1)), 
										formatItem(3, GetItem(iPageIndex, 2)), 
										formatItem(4, GetItem(iPageIndex, 3)), 
										formatItem(5, GetItem(iPageIndex, 4)), 
										formatItem(6, GetItem(iPageIndex, 5)), 
										formatItem(7, bDrawPreview ? "上一页" : ""), 
										formatItem(8, bDrawNext ? "下一页" : ""));
	// clang-format on

	pMenuEntity->SetText(sMenuText.c_str());
	pMenuEntity->Enable();

	Vector& vmPos = pViewModel->GetAbsOrigin();
	QAngle& vmAng = pViewModel->GetAbsAngles();
	Vector panelPos = GetAimPoint(vmPos, vmAng, 7.0f);
	QAngle panelAng = vmAng;
	panelAng.y -= 90.0f;
	panelAng.z += 90.0f;

	Vector rig;
	Vector dwn;
	AngleVectors(panelAng, &rig, &dwn, nullptr);

	rig *= g_fMenuOffsetX;
	dwn *= g_fMenuOffsetY + 1.85f;

	panelPos += rig + dwn;
	pMenuEntity->Teleport(&panelPos, &panelAng, nullptr);

	// background
	CPointWorldText* pMenuBackground = this->m_hBackground.Get();
	if (!pMenuBackground) {
		return;
	}

	Vector bgPos = GetAimPoint(vmPos, vmAng, 7.09f);
	QAngle bgAng = vmAng;
	bgAng.y -= 90.0f;
	bgAng.z += 90.0f;

	AngleVectors(bgAng, &rig, &dwn, nullptr);

	rig *= (g_fMenuOffsetX - 0.2f);
	dwn *= (g_fMenuOffsetY + 1.64f);

	bgPos += rig + dwn;
	pMenuBackground->Teleport(&bgPos, &bgAng, nullptr);

	pMenuBackground->SetParent(pViewModel);
	pMenuBackground->m_hOwnerEntity(pViewModel->GetRefEHandle());

	pMenuBackground->Enable();
}

std::string CBaseMenu::GetItem(int iPageIndex, int iItemIndex) {
	if (iPageIndex < 0 || iPageIndex >= this->m_vItems.size()) {
		SURF_ASSERT(false);
		return "";
	}

	if (iItemIndex < 0) {
		SURF_ASSERT(false);
		return "";
	}

	auto& vItems = m_vItems[iPageIndex];
	if (iItemIndex < vItems.size()) {
		return vItems[iItemIndex];
	}

	return "";
}

CMenuManager g_MenuManager;

CMenuManager* MENU::GetManager() {
	return &g_MenuManager;
}

void CMenuManager::OnMenuItemSelect(CCSPlayerController* pController, const std::vector<std::string>& vArgs) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		SURF_ASSERT(false);
		return;
	}

	if (!pMenuPlayer->m_pCurrentMenu.IsValid()) {
		return;
	}

	auto& pMenu = pMenuPlayer->m_pCurrentMenu.m_pMenu;

	int num = -1;
	if (vArgs.size() > 0) {
		num = V_StringToInt32(vArgs[0].c_str(), -1);
	}

	switch (num) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6: {
			if (pMenu->m_pFnMenuHandler) {
				int iItemIndex = (pMenuPlayer->m_iCurrentPage * CBaseMenu::PAGE_SIZE) + num - 1;
				pMenu->m_pFnMenuHandler(pMenuPlayer->m_pCurrentMenu, pController, iItemIndex);
			}

			break;
		}
		case 7: {
			int iPrevPageIndex = pMenuPlayer->m_iCurrentPage - 1;
			if (iPrevPageIndex >= 0 && iPrevPageIndex < pMenu->GetPageLength()) {
				pMenu->Display(pController->GetPlayerPawn(), iPrevPageIndex);
			}
			break;
		}
		case 8: {
			int iNextPageIndex = pMenuPlayer->m_iCurrentPage + 1;
			if (iNextPageIndex >= 0 && iNextPageIndex < pMenu->GetPageLength()) {
				pMenu->Display(pController->GetPlayerPawn(), iNextPageIndex);
			}
			break;
		}
		case 9: {
			// prev, next menu not impl yet.
			pMenuPlayer->m_pCurrentMenu.Free();
			break;
		}
	}
}

void CMenuManager::OnPluginStart() {
	CONCMD::RegConsoleCmd("sm_menu_select", OnMenuItemSelect);
}

CBaseMenu* MENU::Create(MenuHandler pMenuHandler, EMenuType eMenuType) {
	CBaseMenu* pMenu = nullptr;

	switch (eMenuType) {
		case EMenuType::POINT_WORLDTEXT: {
			pMenu = new CWorldTextMenu(pMenuHandler);
			break;
		}
	}

	return pMenu;
}
