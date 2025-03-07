#include "menu.h"
#include <core/memory.h>
#include <core/concmdmanager.h>
#include <utils/utils.h>
#include <sdk/entity/cbaseentity.h>

#include <fmt/format.h>

extern void SetMenuEntityTransmiter(CBaseEntity* pMenu, CBasePlayerController* pOwner);

constexpr float g_fMenuOffsetX = -9.1f;
constexpr float g_fMenuOffsetY = -4.6f;

Vector CWorldTextMenu::GetRelativeOrigin(const Vector& eyePosition, float distanceToTarget) {
	return Vector(eyePosition.x + distanceToTarget, eyePosition.y, eyePosition.z);
}

CWorldTextMenu::CWorldTextMenu(MenuHandler pFnHandler, std::string sTitle) : CBaseMenu(pFnHandler, sTitle) {
	CPointWorldText* pMenuEntity = (CPointWorldText*)MEM::CALL::CreateEntityByName("point_worldtext");
	if (!pMenuEntity) {
		SDK_ASSERT(false);
		return;
	}

	CEntityKeyValues* pMenuKV = new CEntityKeyValues();
	if (!pMenuKV) {
		pMenuEntity->Kill();
		SDK_ASSERT(false);
		return;
	}

	int fontSize = 40;
	pMenuKV->SetBool("enabled", false);
	pMenuKV->SetFloat("world_units_per_pixel", (0.25 / 1000) * fontSize);
	pMenuKV->SetFloat("depth_render_offset", 0.125);
	pMenuKV->SetInt("justify_horizontal", 0);
	pMenuKV->SetInt("justify_vertical", 1);
	pMenuKV->SetInt("reorient_mode", 0);
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

	int bgFontSize = 80;
	pBackgroundKV->SetColor("color", Color(50, 50, 50, 100));
	pBackgroundKV->SetBool("enabled", false);
	pBackgroundKV->SetFloat("world_units_per_pixel", (0.25 / 300) * bgFontSize);
	pBackgroundKV->SetFloat("depth_render_offset", 0.125);
	pBackgroundKV->SetInt("justify_horizontal", 0);
	pBackgroundKV->SetInt("justify_vertical", 1);
	pBackgroundKV->SetInt("reorient_mode", 0);
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
		SDK_ASSERT(false);
		return;
	}

	pMenuPlayer->m_pCurrentMenu.m_pMenu = this;
	pMenuPlayer->m_iCurrentPage = iPageIndex;

	CPointWorldText* pMenuEntity = this->m_hWorldText.Get();
	if (!pMenuEntity) {
		SDK_ASSERT(false);
		return;
	}

	CBaseViewModel* pViewModel = pPawn->GetCustomViewModel();
	if (!pViewModel) {
		SDK_ASSERT(false);
		return;
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
	SetMenuEntityTransmiter(pMenuEntity, pPawn->GetController());

	Vector& vmPos = pViewModel->GetAbsOrigin();
	Vector panelPos = GetRelativeOrigin(vmPos, 7.0f);

	Vector rig;
	Vector dwn;
	static QAngle panelAng = {0.0f, -90.0f, 90.0f};
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

	pMenuBackground->SetParent(pViewModel);
	pMenuBackground->m_hOwnerEntity(pViewModel->GetRefEHandle());

	pMenuBackground->Enable();
	SetMenuEntityTransmiter(pMenuBackground, pPawn->GetController());

	Vector bgPos = GetRelativeOrigin(vmPos, 7.09f);
	AngleVectors(panelAng, &rig, &dwn, nullptr);

	rig *= (g_fMenuOffsetX - 0.2f);
	dwn *= (g_fMenuOffsetY + 1.64f);

	bgPos += rig + dwn;
	pMenuBackground->Teleport(&bgPos, &panelAng, nullptr);
}

std::string CBaseMenu::GetItem(int iPageIndex, int iItemIndex) {
	if (iPageIndex < 0 || iPageIndex >= this->m_vItems.size()) {
		SDK_ASSERT(false);
		return "";
	}

	if (iItemIndex < 0) {
		SDK_ASSERT(false);
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
		SDK_ASSERT(false);
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
