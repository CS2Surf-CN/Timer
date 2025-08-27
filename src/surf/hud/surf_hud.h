#pragma once

#include <surf/surf_player.h>
#include <surf/datatypes.h>
#include <core/screentext.h>

class CSurfHudPlugin : CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;

private:
	void UpdateHudData(CSurfPlayer* pSurfPlayer);
	void UpdateHudDataPost(CSurfPlayer* pSurfPlayer);
	void UpdateHTML(CSurfPlayer* pSurfPlayer);
	void UpdateScreenText(CSurfPlayer* pSurfPlayer);

public:
	static void OnPlayerSpawn(IGameEvent* pEvent, const char* szName, bool bDontBroadcast);
};

class CSurfHudService : CSurfPlayerService {
public:
	using CSurfPlayerService::CSurfPlayerService;

	virtual void OnPlayerQuit() override;

public:
	hudsettings_t m_HUDSettigs;
	i32 m_iCurrentSpeed;
	i32 m_iPrevousSpeed;
	std::weak_ptr<CScreenText> m_wpSpeedText;
};

namespace SURF {
	extern CSurfHudPlugin* HudPlugin();
} // namespace SURF
