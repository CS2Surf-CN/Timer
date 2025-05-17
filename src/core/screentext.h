#pragma once

#include <pch.h>
#include <core/playermanager.h>
#include <movement/movement.h>
#include <list>

struct ScreenTextManifest_t {
	Vector2D m_vecPos = {0.0f, 0.0f};
	Color m_Color = {0, 222, 101, 255};
	int m_iUnits = 300;
	std::string m_sFont = "Trebuchet MS";
	float m_fFontSize = 20.0f;
	float m_fBackgroundBorderWidth = 0.15f;
	float m_fBackgroundBorderHeight = 0.2f;
	bool m_bBackground = false;
	bool m_bEnable = true;
};

class CScreenText {
public:
	CScreenText(const ScreenTextManifest_t& manifest);
	~CScreenText();

	void SetText(const std::string_view& text);

	// plane coordinate
	//      ^
	//      |
	//  <- 0.0f ->
	//      |
	//      v
	void SetPos(float x, float y);

	void SetColor(Color color);
	void SetFont(const std::string_view& font);
	void SetFontSize(float fontsize);
	void SetUnits(int unit);

	bool IsRendering();

	// FIXME: dead, spectator not supported yet.
	void Display(CBasePlayerController* pController);

	void UpdatePos();

	void Enable() {
		m_hScreenEnt->Enable();
	}

	void Disable() {
		m_hScreenEnt->Disable();
	}

	CBasePlayerController* GetOriginalController() const {
		return m_hOriginalController.IsValid() ? m_hOriginalController.Get() : nullptr;
	}

public:
	static Vector GetRelativeVMOrigin(const Vector& eyePosition, float distanceToTarget = 6.7f);
	static Vector GetRelativePawnOrigin(const Vector& eyePosition, const QAngle& eyeAngles, float distanceToTarget = 7.09f);

public:
	Vector2D m_vecPos;

private:
	CHandle<CPointWorldText> m_hScreenEnt;
	CHandle<CBasePlayerPawn> m_hOwner;

public:
	CHandle<CBasePlayerController> m_hOriginalController;
};

class CScreenTextController : public CPlayer {
public:
	using CPlayer::CPlayer;

	virtual void Reset() override {
		CPlayer::Reset();

		m_ScreenTextList.clear();
	}

public:
	std::list<std::shared_ptr<CScreenText>> m_ScreenTextList;
};

class CScreenTextControllerManager : CPlayerManager, CMovementForward {
public:
	CScreenTextControllerManager() {
		for (int i = 0; i < MAXPLAYERS; i++) {
			m_pPlayers[i] = std::make_unique<CScreenTextController>(i);
		}
	}

	virtual CScreenTextController* ToPlayer(CBasePlayerController* controller) const override {
		return static_cast<CScreenTextController*>(CPlayerManager::ToPlayer(controller));
	}

	virtual CScreenTextController* ToPlayer(CBasePlayerPawn* pawn) const override {
		return static_cast<CScreenTextController*>(CPlayerManager::ToPlayer(pawn));
	}

private:
	virtual void OnPluginStart() override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawn* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;
};

namespace VGUI {
	CScreenTextControllerManager* GetScreenTextManager();

	[[nodiscard]] std::weak_ptr<CScreenText> CreateScreenText(CBasePlayerController* pController, std::optional<ScreenTextManifest_t> manifest = std::nullopt);
	void Render(const std::weak_ptr<CScreenText>& hText);
	void Unrender(const std::weak_ptr<CScreenText>& hText);
	void Dispose(const std::weak_ptr<CScreenText>& hText);
	void Cleanup(CBasePlayerController* pController);
} // namespace VGUI
