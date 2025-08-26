#include "menu.h"
#include <core/memory.h>
#include <core/concmdmanager.h>
#include <core/eventmanager.h>
#include <utils/utils.h>
#include <sdk/entity/cbaseentity.h>
#include <fmt/format.h>

// constexpr float g_fMenuDefaultOffsetX_Alive = -8.9f;
// constexpr float g_fMenuDefaultOffsetY_Alive = -0.3f;
constexpr float g_fMenuDefaultOffsetX_Alive = -9.3f;
constexpr float g_fMenuDefaultOffsetY_Alive = 0.5f;

CScreenTextMenu::CScreenTextMenu(CBasePlayerController* pController, std::string sTitle)
	: CBaseMenu(pController, sTitle) {
	ScreenTextManifest_t manifest;
	manifest.m_iUnits = 1000;
	manifest.m_sFont = "Arial Bold";
	manifest.m_Color = Color(255, 165, 0, 255);
	manifest.m_fFontSize = 40;
	manifest.m_bEnable = false;
	manifest.m_vecPos.x = g_fMenuDefaultOffsetX_Alive;
	manifest.m_vecPos.y = g_fMenuDefaultOffsetY_Alive;
	manifest.m_bBackground = true;

	m_wpScreenText = VGUI::CreateScreenText(pController, manifest);
}

CScreenTextMenu::~CScreenTextMenu() {
	CBaseMenu::~CBaseMenu();
	this->Close();
}

bool CBaseMenu::Display(int iPageIndex) {
	if (m_bDisplayed) {
		return true;
	}

	auto pController = m_hController.Get();
	if (!pController) {
		Assert(false);
		return false;
	}

	auto pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return false;
	}

	auto& pCurrentMenu = pMenuPlayer->GetCurrentMenu();

	Assert(pCurrentMenu.get() == this);

	CMenuHandle hMenu(pCurrentMenu);
	MenuEvent_t event(hMenu, m_hController.Get(), -1);
	if (pCurrentMenu->m_pFnOnDisplay) {
		pCurrentMenu->m_pFnOnDisplay(event);
	}

	if (!hMenu) {
		auto& pPrevMenu = pMenuPlayer->GetPreviousMenu();
		if (pPrevMenu) {
			pPrevMenu->Display();
		}
		return false;
	}

	m_bDisplayed = true;

	return true;
}

bool CScreenTextMenu::Display(int iPageIndex) {
	if (!Super::Display(iPageIndex)) {
		return false;
	}

	if (m_wpScreenText.expired()) {
		Assert(false);
		return false;
	}

	auto pMenuText = m_wpScreenText.lock().get();
	auto pController = pMenuText->GetOriginalController();
	if (!pController) {
		return false;
	}

	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return false;
	}

	auto& pCurrentMenu = pMenuPlayer->GetCurrentMenu();
	for (const auto& pMenu : pMenuPlayer->m_MenuQueue) {
		if (pMenu != pCurrentMenu) {
			pMenu->Disable();
		}
	}

	pMenuPlayer->m_iCurrentPage = iPageIndex;
	pMenuPlayer->ClampItemOnPage();
	auto iCurrentPageItem = pMenuPlayer->m_iCurrentItem % CBaseMenu::PAGE_MAXITEMS;
	auto bWSAD = pMenuPlayer->m_bWSADMenu;
	auto nItemLength = GetItemLength();

	if (!nItemLength) {
		this->DisplayEmpty(bWSAD);
		return true;
	}

	bool bDrawPrev = (nItemLength > 0) && (iPageIndex != 0);
	bool bDrawNext = (nItemLength > 0) && (iPageIndex + 1 < static_cast<int>(GetPageSize()));
	bool bDrawPrevNext = bDrawPrev && bDrawNext;

	auto itemRes0 = GetItem(iPageIndex, 0);
	auto itemRes1 = GetItem(iPageIndex, 1);
	auto itemRes2 = GetItem(iPageIndex, 2);
	auto itemRes3 = GetItem(iPageIndex, 3);
	auto itemRes4 = GetItem(iPageIndex, 4);
	auto itemRes5 = GetItem(iPageIndex, 5);

	auto formatItem = [iCurrentPageItem, bWSAD](int iItemIndex, const std::string& sItem) -> std::string {
		if (sItem.empty()) {
			return "";
		}

		if (bWSAD) {
			return fmt::format("{}{}", iItemIndex == iCurrentPageItem ? "> " : "", sItem);
		}

		return fmt::format("{}.{}", iItemIndex + 1, sItem);
	};

	// clang-format off
	std::string sMenuText = fmt::format("{}\n\n"
										"{}\n"
										"{}\n"
										"{}\n"
										"{}\n"
										"{}\n"
										"{}\n\n"
										"{}",
										this->m_sTitle, 
										formatItem(0, itemRes0.has_value() ? itemRes0.value().get().first : ""), 
										formatItem(1, itemRes1.has_value() ? itemRes1.value().get().first : ""), 
										formatItem(2, itemRes2.has_value() ? itemRes2.value().get().first : ""), 
										formatItem(3, itemRes3.has_value() ? itemRes3.value().get().first : ""), 
										formatItem(4, itemRes4.has_value() ? itemRes4.value().get().first : ""), 
										formatItem(5, itemRes5.has_value() ? itemRes5.value().get().first : ""), 
										!bWSAD ? fmt::format("{}\n"
															 "{}\n"
															 "{}",
															 formatItem(6, bDrawPrev ? "上一页" : ""), 
															 formatItem(7, bDrawNext ? "下一页" : ""),
															 formatItem(8, "退出"))
											   : fmt::format("{}W/S: 滚动\n"
														     "E: 选择, F: 锁定\n"
															 "Shift: 退出", 
															 (bDrawPrevNext ? "A/D: 翻页, " 
															: bDrawPrev ? "A: 上一页, " 
															: bDrawNext ? "D: 下一页, "
															: "")));
	// clang-format on

	pMenuText->SetText(sMenuText.c_str());
	this->Enable();

	return true;
}

void CScreenTextMenu::Enable() {
	Super::Enable();

	VGUI::Render(m_wpScreenText);
}

void CScreenTextMenu::Disable() {
	Super::Disable();

	VGUI::Unrender(m_wpScreenText);
}

bool CScreenTextMenu::Close() {
	if (m_wpScreenText.expired()) {
		return false;
	}

	VGUI::Dispose(m_wpScreenText);

	return true;
}

void CScreenTextMenu::DisplayEmpty(bool bWSAD) {
	auto pMenuText = m_wpScreenText.lock().get();
	std::string sMenuText = fmt::format("{}\n\n"
										"{}",
										this->m_sTitle,
										!bWSAD ? "8.退出" : "Shift: 退出");

	pMenuText->SetText(sMenuText.c_str());
	this->Enable();
}

void CBaseMenu::AddItem(const std::string_view sItem, std::optional<MenuEventHandler> handler) {
	static auto nullEventHandler = MENU_HANDLER_L() {};
	m_vItems.emplace_back(sItem, handler ? handler.value() : nullEventHandler);
}

void CBaseMenu::SetItem(const size_t nItemIndex, const std::string_view sItem, std::optional<MenuEventHandler> handler) {
	auto itemRes = GetItem(nItemIndex);
	if (!itemRes) {
		return;
	}

	auto& refItem = itemRes.value().get();
	refItem.first = sItem;

	if (handler) {
		refItem.second = handler.value();
	}
}

bool CBaseMenu::DeleteItem(const size_t nItemIndex) {
	if (nItemIndex < 0 || nItemIndex >= m_vItems.size()) {
		return false;
	}

	m_vItems.erase(m_vItems.begin() + nItemIndex);
	return true;
}

std::string_view CBaseMenu::GetItemString(const size_t nItemIndex) {
	if (nItemIndex < 0 || nItemIndex >= m_vItems.size()) {
		return "";
	}

	return m_vItems.at(nItemIndex).first;
}

std::optional<std::reference_wrapper<CBaseMenu::MenuItemHandler>> CBaseMenu::GetItem(size_t nPageIndex, size_t nItemActualIndex) {
	if (nPageIndex < 0 || nPageIndex >= this->GetPageSize()) {
		Assert(false);
		return {};
	}

	if (nItemActualIndex < 0 || nItemActualIndex >= PAGE_MAXITEMS) {
		Assert(false);
		return {};
	}

	auto nItemIndex = GetItemGlobalIndex(nPageIndex, nItemActualIndex);
	if (nItemIndex >= m_vItems.size()) {
		return {};
	}

	return std::ref(m_vItems.at(nItemIndex));
}

std::optional<std::reference_wrapper<CBaseMenu::MenuItemHandler>> CBaseMenu::GetItem(size_t nItemIndex) {
	if (nItemIndex < 0 || nItemIndex >= m_vItems.size()) {
		Assert(false);
		return {};
	}

	return std::ref(m_vItems.at(nItemIndex));
}

void CMenuPlayer::Cleanup() {
	m_MenuQueue.clear();
}

void CMenuPlayer::ResetMenu(bool bResetMode) {
	m_iCurrentPage = 0;
	m_iCurrentItem = 0;

	if (bResetMode) {
		m_bWSADMenu = false;
		m_bWSADPref = true;
		m_bWSADLocked = false;
	}
}

CMenuManager g_MenuManager;

CMenuManager* MENU::GetManager() {
	return &g_MenuManager;
}

void CMenuPlayer::SelectMenu() {
	auto pController = GetController();
	if (!pController) {
		return;
	}

	const auto& pMenu = GetCurrentMenu();
	if (!pMenu || !pMenu->GetItemLength()) {
		return;
	}

	auto iMenuType = pMenu->GetType();
	if (iMenuType == EMenuType::Unknown) {
		Assert(false);
		return;
	}

	this->ClampItemOnPage();
	CMenuHandle hMenu(pMenu);

	switch (CBaseMenu::GetItemActualIndex(m_iCurrentItem)) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5: {
			const auto& menuHandlerRes = pMenu->m_vItems.at(m_iCurrentItem).second;
			if (menuHandlerRes) {
				MenuEvent_t event(hMenu, pController, m_iCurrentItem);
				const auto& handler = *menuHandlerRes;
				handler(event);
				if (hMenu && pMenu == GetCurrentMenu()) {
					Refresh();
				}

				// EngineServer()->GetInstance()->ClientCommand(GetPlayerSlot().Get(), fmt::format("play {}", MENU_SND_SELECT_PATH).data());
				UTIL::PlaySoundToClient(GetPlayerSlot(), MENU_SND_SELECT);
			}

			break;
		}
		case 6: {
			this->DisplayPagePrev();
			break;
		}
		case 7: {
			this->DisplayPageNext();
			break;
		}
		case 8: {
			this->Exit();
			break;
		}
	}
}

void CMenuPlayer::SwitchMode(bool bRedraw) {
	m_bWSADMenu = !m_bWSADMenu;

	if (bRedraw) {
		Refresh();
	}
}

void CMenuPlayer::DisplayPagePrev() {
	int iPrevPageIndex = m_iCurrentPage - 1;
	auto& pCurrentMenu = GetCurrentMenu();
	if (iPrevPageIndex >= 0 && iPrevPageIndex < pCurrentMenu->GetPageSize()) {
		m_iCurrentItem = static_cast<int>(CBaseMenu::GetItemStartIndexOnPage(iPrevPageIndex));
		pCurrentMenu->Display(iPrevPageIndex);
		UTIL::PlaySoundToClient(GetPlayerSlot(), MENU_SND_SELECT);
	}
}

void CMenuPlayer::DisplayPageNext() {
	int iNextPageIndex = m_iCurrentPage + 1;
	auto& pCurrentMenu = GetCurrentMenu();
	if (iNextPageIndex >= 0 && iNextPageIndex < pCurrentMenu->GetPageSize()) {
		m_iCurrentItem = static_cast<int>(CBaseMenu::GetItemStartIndexOnPage(iNextPageIndex));
		pCurrentMenu->Display(iNextPageIndex);
		UTIL::PlaySoundToClient(GetPlayerSlot(), MENU_SND_SELECT);
	}
}

void CMenuPlayer::Refresh() {
	auto& pCurrentMenu = GetCurrentMenu();
	if (pCurrentMenu) {
		pCurrentMenu->m_bDisplayed = false;
		pCurrentMenu->Display(m_iCurrentPage);
	}
}

void CMenuPlayer::Exit() {
	const auto& pCurrentMenu = GetCurrentMenu();
	if (pCurrentMenu->m_bExitBack) {
		pCurrentMenu->Disable();

		CloseCurrentMenu();
	} else {
		CloseAllMenu();
	}

	UTIL::PlaySoundToClient(GetPlayerSlot(), MENU_SND_EXIT);
}

void CMenuPlayer::CloseAllMenu() {
	if (m_MenuQueue.empty()) {
		return;
	}

	const auto& pCurrentMenu = GetCurrentMenu();
	CMenuHandle hMenu(pCurrentMenu);
	if (pCurrentMenu->m_pFnOnExit) {
		MenuEvent_t event(hMenu, GetController(), -1);
		pCurrentMenu->m_pFnOnExit(event);
	}

	Cleanup();
	ResetMenu();
}

void CMenuPlayer::CloseCurrentMenu() {
	if (m_MenuQueue.empty()) {
		return;
	}

	const auto& pCurrentMenu = GetCurrentMenu();
	CMenuHandle hMenu(pCurrentMenu);
	if (pCurrentMenu->m_pFnOnExit) {
		MenuEvent_t event(hMenu, GetController(), -1);
		pCurrentMenu->m_pFnOnExit(event);
	}

	m_MenuQueue.pop_back();
	ResetMenu();
	Refresh();
	// UTIL::PlaySoundToClient(GetPlayerSlot(), MENU_SND_EXIT);
}

std::shared_ptr<CBaseMenu>& CMenuPlayer::GetPreviousMenu() {
	if (m_MenuQueue.size() < 2) {
		static std::shared_ptr<CBaseMenu> s_nullMenu;
		return s_nullMenu;
	}

	return *(m_MenuQueue.rbegin() + 1);
}

std::shared_ptr<CBaseMenu>& CMenuPlayer::GetCurrentMenu() {
	if (m_MenuQueue.empty()) {
		static std::shared_ptr<CBaseMenu> s_nullMenu;
		return s_nullMenu;
	}

	return m_MenuQueue.back();
}

bool CMenuPlayer::IsDisplaying() {
	const auto& menu = GetCurrentMenu();
	return menu.get() != nullptr;
}

void CMenuPlayer::ClampItemOnPage() {
	if (!m_bWSADMenu) {
		return;
	}

	auto& pCurrentMenu = GetCurrentMenu();
	int startIdx = static_cast<int>(pCurrentMenu->GetItemStartIndexOnPage(m_iCurrentPage));
	int currentPageItemLength = static_cast<int>(pCurrentMenu->GetItemLengthOnPage(m_iCurrentPage));
	if (m_iCurrentItem < startIdx) {
		m_iCurrentItem = startIdx + currentPageItemLength - 1;
	} else if (m_iCurrentItem >= startIdx + currentPageItemLength) {
		m_iCurrentItem = startIdx;
	}
}

void CMenuManager::OnPlayerSpawn(IGameEvent* pEvent, const char* szName, bool bServerOnly) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	auto hController = pController->GetRefEHandle();

	UTIL::RequestFrame([hController]() {
		CCSPlayerController* pController = static_cast<CCSPlayerController*>(hController.Get());
		if (!pController) {
			return;
		}

		if (!pController->m_bPawnIsAlive()) {
			return;
		}

		CBasePlayerPawn* pPawn = pController->GetCurrentPawn();
		if (!pPawn || !pPawn->IsAlive()) {
			return;
		}

		CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pPawn);
		if (!pMenuPlayer) {
			Assert(false);
			return;
		}

		pMenuPlayer->m_bWSADMenu = pMenuPlayer->m_bWSADPref;
		pMenuPlayer->Refresh();
	});
}

void CMenuManager::OnPlayerTeam(IGameEvent* pEvent, const char* szName, bool bServerOnly) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	auto iNewTeam = pEvent->GetInt("team");
	auto hController = pController->GetRefEHandle();

	UTIL::RequestFrame([hController, iNewTeam]() {
		CCSPlayerController* pController = static_cast<CCSPlayerController*>(hController.Get());
		if (!pController) {
			return;
		}

		CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
		if (!pMenuPlayer) {
			Assert(false);
			return;
		}

		if (iNewTeam == CS_TEAM_SPECTATOR) {
			pMenuPlayer->m_bWSADMenu = true;
			pMenuPlayer->Refresh();
		}
	});
}

void CMenuManager::OnIntermission(IGameEvent* pEvent, const char* szName, bool bServerOnly) {
	auto iMaxPlayers = UTIL::GetGlobals()->maxClients;
	for (auto i = 0; i < iMaxPlayers; i++) {
		CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(CPlayerSlot(i));
		if (!pMenuPlayer) {
			continue;
		}

		// FIXME: Redraw menu on intermission end???
		pMenuPlayer->Cleanup();
	}
}

void CMenuManager::OnPluginStart() {
	// confict with sm menu manager
	// ConCmdManager()->RegConsoleCmd("sm_menu_select", OnMenuItemSelect);

	for (int i = 1; i <= 9; i++) {
		std::string sMenuCmd = fmt::format("sm_{}", i);
		CONCMD::RegConsoleCmd(sMenuCmd, OnNumberSelect);
	}

	CONCMD::RegConsoleCmd("sm_mma", OnMenuModeChange);

	EVENT::HookEvent("player_spawn", OnPlayerSpawn);
	EVENT::HookEvent("player_team", OnPlayerTeam);
	EVENT::HookEvent("cs_intermission", OnIntermission);
}

void CMenuManager::OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pPawn);
	if (!pMenuPlayer) {
		Assert(false);
		return;
	}

	if (!pMenuPlayer->IsDisplaying() || !pMenuPlayer->m_bWSADMenu) {
		return;
	}

	if (buttons.Pressed(IN_USE)) {
		pMenuPlayer->SelectMenu();
	} else if (buttons.Pressed(IN_LOOK_AT_WEAPON)) {
		pMenuPlayer->m_bWSADLocked = !pMenuPlayer->m_bWSADLocked;
	} else if (buttons.Pressed(IN_SPEED)) {
		pMenuPlayer->m_bWSADLocked = false;
		pMenuPlayer->Exit();
	}

	if (pMenuPlayer->m_bWSADLocked) {
		return;
	}

	if (buttons.Pressed(IN_FORWARD)) {
		pMenuPlayer->m_iCurrentItem--;
		pMenuPlayer->Refresh();
	} else if (buttons.Pressed(IN_BACK)) {
		pMenuPlayer->m_iCurrentItem++;
		pMenuPlayer->Refresh();
	} else if (buttons.Pressed(IN_MOVELEFT)) {
		pMenuPlayer->DisplayPagePrev();
	} else if (buttons.Pressed(IN_MOVERIGHT)) {
		pMenuPlayer->DisplayPageNext();
	}
}

void CMenuManager::OnSetObserverTargetPost(CPlayer_ObserverServices* pService, CBaseEntity* pEnt, const ObserverMode_t iObsMode) {
	if (iObsMode == OBS_MODE_IN_EYE || iObsMode == OBS_MODE_CHASE) {
		auto pController = (CCSPlayerController*)pService->GetPawn()->GetController();
		if (!pController) {
			return;
		}

		CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
		if (!pMenuPlayer) {
			SDK_ASSERT(false);
			return;
		}

		pMenuPlayer->m_bWSADMenu = true;
		pMenuPlayer->Refresh();
	}
}

void CMenuManager::OnMenuItemSelect(CCSPlayerController* pController, const std::vector<std::string>& vArgs, const std::wstring& wCommand) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return;
	}

	if (!pMenuPlayer->IsDisplaying()) {
		return;
	}

	int num = -1;
	if (vArgs.size() > 0) {
		num = V_StringToInt32(vArgs[0].c_str(), -1) - 1;
	}

	pMenuPlayer->m_iCurrentItem = num;
	pMenuPlayer->SelectMenu();
}

void CMenuManager::OnNumberSelect(CCSPlayerController* pController, const std::vector<std::string>& vArgs, const std::wstring& wCommand) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return;
	}

	if (!pMenuPlayer->IsDisplaying()) {
		return;
	}

	pMenuPlayer->m_iCurrentItem = wCommand[wCommand.length() - 2] - L'0';
	pMenuPlayer->m_iCurrentItem--;
	pMenuPlayer->SelectMenu();
}

void CMenuManager::OnMenuModeChange(CCSPlayerController* pController, const std::vector<std::string>& vArgs, const std::wstring& wCommand) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return;
	}

	pMenuPlayer->SwitchMode(true);
	pMenuPlayer->m_bWSADPref = pMenuPlayer->m_bWSADMenu;
}

CMenuHandle MENU::Create(CBasePlayerController* pController, EMenuType eMenuType) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return {};
	}

	switch (eMenuType) {
		case EMenuType::ScreenText: {
			pMenuPlayer->ResetMenu();

			auto pMenu = std::make_shared<CScreenTextMenu>(pController);
			pMenuPlayer->m_MenuQueue.emplace_back(pMenu);
			return CMenuHandle(pMenu);
		}
	}

	return {};
}

bool MENU::CloseAll(CBasePlayerController* pController) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return false;
	}

	pMenuPlayer->CloseAllMenu();
	return true;
}

bool MENU::CloseCurrent(CBasePlayerController* pController) {
	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return false;
	}

	pMenuPlayer->CloseCurrentMenu();
	return true;
}

bool CMenuHandle::CloseAll() {
	if (!IsValid()) {
		return false;
	}

	auto pController = Data()->GetController();
	if (!pController) {
		Assert(false);
		return false;
	}

	return MENU::CloseAll(pController);
}

bool CMenuHandle::CloseCurrent() {
	if (!IsValid()) {
		return false;
	}

	auto pController = Data()->GetController();
	if (!pController) {
		Assert(false);
		return false;
	}

	if (!MENU::CloseCurrent(pController)) {
		return false;
	}

	return true;
}

CMenuHandle CMenuHandle::GetPreviousMenu() {
	if (!IsValid()) {
		return {};
	}

	auto pController = Data()->GetController();
	if (!pController) {
		Assert(false);
		return {};
	}

	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return {};
	}

	return CMenuHandle(pMenuPlayer->GetPreviousMenu());
}

void CMenuHandle::Refresh() {
	if (!IsValid()) {
		return;
	}

	auto pController = Data()->GetController();
	if (!pController) {
		Assert(false);
		return;
	}

	CMenuPlayer* pMenuPlayer = MENU::GetManager()->ToPlayer(pController);
	if (!pMenuPlayer) {
		Assert(false);
		return;
	}

	pMenuPlayer->Refresh();
}
