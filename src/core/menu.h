#pragma once

#include <core/playermanager.h>
#include <core/screentext.h>
#include <movement/movement.h>
#include <deque>

constexpr auto MENU_SND_SELECT = "UIPanorama.submenu_select";
constexpr auto MENU_SND_EXIT = "UIPanorama.submenu_slidein";

constexpr auto MENU_SND_SELECT_PATH = "sounds/ui/panorama/submenu_dropdown_select_01";
constexpr auto MENU_SND_EXIT_PATH = "sounds/ui/panorama/submenu_slidein_01";

enum class EMenuType {
	Unknown = 0,
	ScreenText
};

class CMenuHandle : public CStdWeakHandle<class IBaseMenu> {
public:
	using CStdWeakHandle::CStdWeakHandle;

	bool CloseAll();

	// Close current menu and display previous
	bool CloseCurrent();

	CMenuHandle GetPreviousMenu();

	void Refresh();
};

struct MenuEvent_t {
	MenuEvent_t(CMenuHandle& hMenu, CBasePlayerController* pController, size_t nItem) {
		this->hMenu = hMenu;
		this->pController = pController;
		this->nItem = nItem;
	}

	CMenuHandle hMenu;
	CBasePlayerController* pController;
	size_t nItem;
};

#define MENU_HANDLER_ARGS   MenuEvent_t& event
#define MENU_HANDLER(fn)    static void fn(MENU_HANDLER_ARGS)
#define MENU_HANDLER_L(...) [__VA_ARGS__](MENU_HANDLER_ARGS) mutable -> void
using MenuEventHandler = std::function<void(MENU_HANDLER_ARGS)>;

class IBaseMenu {
protected:
	virtual ~IBaseMenu() {};

public:
	virtual EMenuType GetType() = 0;
	virtual CBasePlayerController* GetController() = 0;
	virtual bool Display(int iPageIndex = 0) = 0;
	virtual void Enable() = 0;
	virtual void Disable() = 0;
	virtual bool Close() = 0;
	virtual void SetTitle(const std::string_view sTitle) = 0;
	virtual void SetExitback(bool bExitback) = 0;
	virtual void SetOnDisplay(const MenuEventHandler& handler) = 0;
	virtual void SetOnExit(const MenuEventHandler& handler) = 0;
	virtual void AddItem(const std::string_view sItem, std::optional<MenuEventHandler> handler = std::nullopt) = 0;
	virtual void SetItem(const size_t nItemIndex, const std::string_view sItem, std::optional<MenuEventHandler> handler = std::nullopt) = 0;
	virtual bool DeleteItem(const size_t nItemIndex) = 0;
	virtual void ClearItem() = 0;
	virtual std::string_view GetItemString(const size_t nItemIndex) = 0;
};

class CBaseMenu : public IBaseMenu {
public:
	using MenuItemHandler = std::pair<std::string, std::optional<MenuEventHandler>>;
	static inline const size_t PAGE_MAXITEMS = 6;
	friend class CMenuPlayer;

public:
	CBaseMenu(CBasePlayerController* pController, std::string sTitle = "")
		: m_sTitle(sTitle) {
		m_hController = pController->GetRefEHandle();
	}

	virtual EMenuType GetType() override {
		return EMenuType::Unknown;
	}

	virtual CBasePlayerController* GetController() override {
		return m_hController.Get();
	}

	virtual bool Display(int iPageIndex = 0) override;

	virtual void Enable() override {
		m_bDisplayed = true;
	}

	virtual void Disable() override {
		m_bDisplayed = false;
	}

public:
	virtual void SetTitle(const std::string_view sTitle) override {
		m_sTitle = sTitle;
	}

	virtual void SetExitback(bool bExitback) override {
		m_bExitBack = bExitback;
	}

	virtual void SetOnDisplay(const MenuEventHandler& handler) override {
		m_pFnOnDisplay = handler;
	}

	virtual void SetOnExit(const MenuEventHandler& handler) override {
		m_pFnOnExit = handler;
	}

	virtual void AddItem(const std::string_view sItem, std::optional<MenuEventHandler> handler = std::nullopt) override;
	virtual void SetItem(const size_t nItemIndex, const std::string_view sItem, std::optional<MenuEventHandler> handler = std::nullopt) override;
	virtual bool DeleteItem(const size_t nItemIndex) override;
	virtual std::string_view GetItemString(const size_t nItemIndex) override;

	virtual void ClearItem() override {
		m_vItems.clear();
	}

public:
	size_t GetItemLength() const {
		return m_vItems.size();
	}

	size_t GetItemLengthOnPage(size_t nPageIndex) const {
		size_t startIdx = GetItemStartIndexOnPage(nPageIndex);
		if (startIdx >= m_vItems.size()) {
			return 0;
		}
		return std::min(PAGE_MAXITEMS, (m_vItems.size() - startIdx));
	}

	size_t GetPageSize() const {
		return (m_vItems.size() + PAGE_MAXITEMS - 1) / PAGE_MAXITEMS;
	}

	// nItemActualIndex: 0 ~ PAGE_MAXITEMS
	std::optional<std::reference_wrapper<MenuItemHandler>> GetItem(size_t nPageIndex, size_t nItemActualIndex);
	std::optional<std::reference_wrapper<MenuItemHandler>> GetItem(size_t nItemIndex);

public:
	static size_t GetItemActualIndex(size_t nItemIndex) {
		return nItemIndex % PAGE_MAXITEMS;
	}

	static size_t GetItemStartIndexOnPage(size_t nPageIndex) {
		return nPageIndex * PAGE_MAXITEMS;
	}

	// nItemActualIndex: 0 ~ PAGE_MAXITEMS
	static size_t GetItemGlobalIndex(size_t nPageIndex, size_t nItemActualIndex) {
		return GetItemStartIndexOnPage(nPageIndex) + nItemActualIndex;
	}

	static size_t GetPageIndex(size_t nItemIndex) {
		return nItemIndex / PAGE_MAXITEMS;
	}

public:
	std::string m_sTitle;
	std::vector<MenuItemHandler> m_vItems;
	CHandle<CBasePlayerController> m_hController;
	bool m_bExitBack = false;
	bool m_bDisplayed = false;
	MenuEventHandler m_pFnOnDisplay;
	MenuEventHandler m_pFnOnExit;
};

class CScreenTextMenu : public CBaseMenu {
	using Super = CBaseMenu;

public:
	CScreenTextMenu(CBasePlayerController* pController, std::string sTitle = "");

	virtual EMenuType GetType() override {
		return EMenuType::ScreenText;
	}

public:
	virtual ~CScreenTextMenu() override;
	virtual bool Display(int iPageIndex = 0) override;
	virtual void Enable() override;
	virtual void Disable() override;
	virtual bool Close() override;

private:
	void DisplayEmpty(bool bWSAD);

public:
	std::weak_ptr<CScreenText> m_wpScreenText;
};

class CMenuPlayer : public CPlayer {
private:
	virtual void Reset() override {
		CPlayer::Reset();

		Cleanup();
		ResetMenu(true);
	}

public:
	using CPlayer::CPlayer;

	void Cleanup();
	void ResetMenu(bool bResetMode = false);
	void SelectMenu();
	void SwitchMode(bool bRedraw = false);
	void DisplayPagePrev();
	void DisplayPageNext();
	void Refresh();
	void Exit();
	void CloseAllMenu();
	void CloseCurrentMenu();

	std::shared_ptr<CBaseMenu>& GetPreviousMenu();
	std::shared_ptr<CBaseMenu>& GetCurrentMenu();
	bool IsDisplaying();

private:
	friend class CScreenTextMenu;
	void ClampItemOnPage();

public:
	std::deque<std::shared_ptr<CBaseMenu>> m_MenuQueue;
	int m_iCurrentPage {};
	int m_iCurrentItem {};
	bool m_bWSADMenu {};
	bool m_bWSADPref = true;
	bool m_bWSADLocked {};
};

class CMenuManager : CPlayerManager, CMovementForward, CFeatureForward {
	using Super = CPlayerManager;

public:
	CMenuManager() {
		for (int i = 0; i < MAXPLAYERS; i++) {
			m_pPlayers[i] = std::make_unique<CMenuPlayer>(i);
		}
	}

	virtual CMenuPlayer* ToPlayer(CBasePlayerController* controller) const override {
		return static_cast<CMenuPlayer*>(Super::ToPlayer(controller));
	}

	virtual CMenuPlayer* ToPlayer(CBasePlayerPawn* pawn) const override {
		return static_cast<CMenuPlayer*>(Super::ToPlayer(pawn));
	}

	virtual CMenuPlayer* ToPlayer(CPlayerSlot slot) const override {
		return static_cast<CMenuPlayer*>(Super::ToPlayer(slot));
	}

public:
	bool IsOpeningMenu(CBasePlayerController* controller) const {
		return ToPlayer(controller)->GetCurrentMenu() != nullptr;
	}

	bool IsOpeningMenu(CBasePlayerPawn* pawn) const {
		return ToPlayer(pawn)->GetCurrentMenu() != nullptr;
	}

	bool IsOpeningMenu(CPlayerSlot slot) const {
		return ToPlayer(slot)->GetCurrentMenu() != nullptr;
	}

private:
	virtual void OnPluginStart() override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;
	virtual void OnSetObserverTargetPost(CPlayer_ObserverServices* pService, CBaseEntity* pEnt, const ObserverMode_t iObsMode) override;

	static void OnMenuItemSelect(CCSPlayerController* pController, const std::vector<std::string>& vArgs, const std::wstring& wCommand);
	static void OnNumberSelect(CCSPlayerController* pController, const std::vector<std::string>& vArgs, const std::wstring& wCommand);
	static void OnMenuModeChange(CCSPlayerController* pController, const std::vector<std::string>& vArgs, const std::wstring& wCommand);

	static void OnPlayerSpawn(IGameEvent* pEvent, const char* szName, bool bServerOnly);
	static void OnPlayerTeam(IGameEvent* pEvent, const char* szName, bool bServerOnly);
	static void OnIntermission(IGameEvent* pEvent, const char* szName, bool bServerOnly);
};

namespace MENU {
	extern CMenuManager* GetManager();

	[[nodiscard]] CMenuHandle Create(CBasePlayerController* pController, EMenuType eMenuType = EMenuType::ScreenText);
	bool CloseAll(CBasePlayerController* pController);
	bool CloseCurrent(CBasePlayerController* pController);
} // namespace MENU
