#include <core/screentext.h>
#include <core/memory.h>

CScreenTextControllerManager g_ScreenTextControllerManager;

extern void SetScreenTextEntityTransmiter(CBaseEntity* pScreenEnt, CBasePlayerController* pOwner);

CScreenText::CScreenText(const ScreenTextManifest_t& manifest) : m_vecPos(manifest.m_vecPos) {
	CPointWorldText* pText = (CPointWorldText*)MEM::CALL::CreateEntityByName("point_worldtext");
	if (!pText) {
		SDK_ASSERT(false);
		return;
	}

	pText->m_Color(manifest.m_Color);
	pText->m_FontName(manifest.m_sFont.c_str());
	pText->m_flFontSize(manifest.m_fFontSize);
	pText->m_flWorldUnitsPerPx((0.25 / manifest.m_iUnits) * manifest.m_fFontSize);
	pText->m_flDepthOffset(0.0f);
	pText->m_fadeMinDist(0.0f);
	pText->m_fadeMinDist(0.0f);
	pText->m_nJustifyHorizontal(PointWorldTextJustifyHorizontal_t::POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_LEFT);
	pText->m_nJustifyVertical(PointWorldTextJustifyVertical_t::POINT_WORLD_TEXT_JUSTIFY_VERTICAL_CENTER);
	pText->m_bFullbright(true);
	pText->m_bDrawBackground(manifest.m_bBackground);
	pText->m_flBackgroundBorderHeight(manifest.m_fBackgroundBorderHeight);
	pText->m_flBackgroundBorderWidth(manifest.m_fBackgroundBorderWidth);

	pText->m_messageText("Sample Text");
	pText->m_bEnabled(manifest.m_bEnable);

	pText->DispatchSpawn();

	this->m_hScreenEnt.Set(pText);
}

CScreenText::~CScreenText() {
	if (m_hScreenEnt.IsValid()) {
		m_hScreenEnt->Kill();
		m_hScreenEnt.Term();
	}

	m_hOwner.Term();
	m_hOriginalController.Term();
}

void CScreenText::SetText(const std::string_view& text) {
	m_hScreenEnt->SetText(text.data());
}

void CScreenText::SetPos(float x, float y) {
	m_vecPos.x = x;
	m_vecPos.y = y;

	UpdatePos();
}

void CScreenText::SetColor(Color color) {
	m_hScreenEnt->m_Color(color);
}

void CScreenText::SetFont(const std::string_view& font) {
	m_hScreenEnt->m_FontName(font.data());
}

void CScreenText::SetFontSize(float fontsize) {
	m_hScreenEnt->m_flFontSize(fontsize);
}

void CScreenText::SetUnits(int unit) {
	m_hScreenEnt->m_flWorldUnitsPerPx((0.25 / unit) * m_hScreenEnt->m_flFontSize());
}

bool CScreenText::IsRendering() {
	return m_hOwner.IsValid() && m_hScreenEnt->m_bEnabled();
}

void CScreenText::Display(CBasePlayerController* pController) {
	// Already displayed
	if (IsRendering()) {
		return;
	}

	CPointWorldText* pText = this->m_hScreenEnt.Get();
	if (!pText) {
		SDK_ASSERT(false);
		return;
	}

	// FIXME: check if pawn is observer
	CCSPlayerPawnBase* pPawn = dynamic_cast<CCSPlayerPawnBase*>(pController->GetCurrentPawn());
	if (!pPawn) {
		SDK_ASSERT(false);
		return;
	}

	CBaseViewModel* pViewModel = pPawn->GetCustomViewModel();
	if (!pViewModel) {
		SDK_ASSERT(false);
		return;
	}

	pText->SetParent(pViewModel);
	pText->m_hOwnerEntity(pViewModel->GetRefEHandle());
	m_hOwner.Set(pPawn);

	pText->Enable();
	SetScreenTextEntityTransmiter(pText, pPawn->GetController());

	UpdatePos();
}

void CScreenText::UpdatePos() {
	auto pText = m_hScreenEnt.Get();

	if (!pText || !pText->m_hOwnerEntity().IsValid()) {
		return;
	}

	CBaseViewModel* pViewModel = (CBaseViewModel*)pText->m_hOwnerEntity().Get();

	Vector& vmPos = pViewModel->GetAbsOrigin();
	Vector panelPos = GetRelativeOrigin(vmPos);

	Vector rig;
	Vector up;
	static QAngle panelAng = {0.0f, -90.0f, 90.0f};
	AngleVectors(panelAng, &rig, &up, nullptr);

	rig *= m_vecPos.x;
	up *= m_vecPos.y * -1.0f;

	panelPos += rig + up;
	pText->Teleport(&panelPos, &panelAng, nullptr);
}

Vector CScreenText::GetRelativeOrigin(const Vector& eyePosition, float distanceToTarget) {
	return Vector(eyePosition.x + distanceToTarget, eyePosition.y, eyePosition.z);
}

CScreenTextControllerManager* VGUI::GetScreenTextManager() {
	return &g_ScreenTextControllerManager;
}

std::weak_ptr<CScreenText> VGUI::CreateScreenText(CBasePlayerController* pController, std::optional<ScreenTextManifest_t> manifest) {
	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		SDK_ASSERT(false);
		return {};
	}

	if (!manifest) {
		manifest = ScreenTextManifest_t {};
	}

	auto pText = std::make_shared<CScreenText>(manifest.value());
	pText->m_hOriginalController = pController->GetRefEHandle();
	pTextController->m_ScreenTextList.emplace_back(pText);
	return pText;
}

void VGUI::Render(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (pText->IsRendering()) {
		return;
	}

	if (!pText->m_hOriginalController.IsValid()) {
		return;
	}

	auto pController = pText->m_hOriginalController.Get();
	if (!pController) {
		return;
	}

	pText->Display(pController);
}

void VGUI::Unrender(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (!pText->IsRendering()) {
		return;
	}

	pText->Disable();
}

void VGUI::Dispose(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (!pText->m_hOriginalController.IsValid()) {
		return;
	}

	auto pController = pText->m_hOriginalController.Get();
	if (!pController) {
		return;
	}

	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		SDK_ASSERT(false);
		return;
	}

	std::erase(pTextController->m_ScreenTextList, pText);
}

void VGUI::Cleanup(CBasePlayerController* pController) {
	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		SDK_ASSERT(false);
		return;
	}

	pTextController->m_ScreenTextList.clear();
}
