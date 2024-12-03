#include "menu.h"
#include <core/memory.h>
#include <core/concmdmanager.h>
#include <utils/utils.h>
#include <sdk/entity/cbaseentity.h>

constexpr float g_fMenuOffsetX = -11.8f;
constexpr float g_fMenuOffsetY = -6.4f;

static Vector GetAimPoint(const Vector& eyePosition, const QAngle& eyeAngles, float distanceToTarget = 100.0) {
	double pitch = eyeAngles.x * (M_PI / 180.0);
	double yaw = eyeAngles.y * (M_PI / 180.0);

	double targetX = eyePosition.x + distanceToTarget * std::cos(pitch) * std::cos(yaw);
	double targetY = eyePosition.y + distanceToTarget * std::cos(pitch) * std::sin(yaw);
	double targetZ = eyePosition.z - distanceToTarget * std::sin(pitch);

	return Vector(targetX, targetY, targetZ);
}

CCMD_CALLBACK(MENU_TEST) {
	auto pPawn = pController->GetPlayerPawn();

	CPointWorldText* menuEntity = (CPointWorldText*)MEM::CALL::CreateEntityByName("point_worldtext");
	if (!menuEntity) {
		SURF_ASSERT(false);
		return;
	}

	CEntityKeyValues* kv = new CEntityKeyValues();
	if (!kv) {
		menuEntity->Kill();
		SURF_ASSERT(false);
		return;
	}

	int fontSize = 40;
	kv->SetColor("color", Color(255, 255, 255, 255));
	kv->SetBool("enabled", true);
	kv->SetFloat("world_units_per_pixel", 0.00025 * fontSize);
	kv->SetFloat("depth_render_offset", 0.125);
	kv->SetInt("justify_horizontal", 0); // 0代表左对齐
	kv->SetInt("justify_vertical", 1);   // 1代表垂直居中
	kv->SetInt("reorient_mode", 0);      // don't change
	kv->SetInt("fullbright", 1);
	kv->SetFloat("font_size", fontSize);
	kv->SetString("font_name", "Consolas");

	CBaseViewModel* pViewModel = (CBaseViewModel*)MEM::CALL::CreateEntityByName("csgo_viewmodel");
	pPawn->m_pViewModelServices()->SetViewModel(1, pViewModel);
	Vector& vmPos = pViewModel->GetAbsOrigin();
	QAngle& vmAng = pViewModel->GetAbsAngles();
	Vector panelPos = GetAimPoint(vmPos, vmAng, 7.15f);
	QAngle panelAng = vmAng;
	panelAng.y -= 90.0f;
	panelAng.z += 90.0f;
	kv->SetQAngle("angles", panelAng);

	menuEntity->DispatchSpawn(kv);

	static auto fnSetParent = [](CBaseEntity* pThis, CBaseEntity* parent) {
		static void* fn = libmem::SignScan("4D 8B D9 48 85 D2", LIB::server);
		MEM::SDKCall<void>(fn, pThis, parent, 0, 0);
	};

	fnSetParent(menuEntity, pViewModel);

	menuEntity->m_hOwnerEntity(pViewModel->GetRefEHandle());

	constexpr const char* s_pszTestMsg = "1.fk valve\n2.fk valve\n3.fk valve\n4.fk valve\n5.fk valve\n6.fk valve\n\n7.上一页\n8.下一页\n\n9.退出";

	menuEntity->SetText(s_pszTestMsg);

	auto countNewlines = [](const char* str) -> int {
		int count = 0;
		while (*str) {
			if (*str == '\n') {
				count++;
			}
			str++;
		}
		return count;
	};

	Vector rig;
	Vector dwn;
	AngleVectors(panelAng, &rig, &dwn, nullptr);

	rig *= g_fMenuOffsetX;
	dwn *= g_fMenuOffsetY + 2.0f;

	panelPos += rig + dwn;
	menuEntity->Teleport(&panelPos, nullptr, nullptr);

	// background
	CPointWorldText* bgEntity = (CPointWorldText*)MEM::CALL::CreateEntityByName("point_worldtext");
	if (!bgEntity) {
		return;
	}

	auto bgKV = new CEntityKeyValues();
	if (!bgKV) {
		bgEntity->Kill();
		return;
	}

	int bgFontSize = 80;
	bgKV->SetColor("color", Color(50, 50, 50, 210));
	bgKV->SetBool("enabled", true);
	bgKV->SetFloat("world_units_per_pixel", (0.25 / 300) * bgFontSize);
	bgKV->SetFloat("depth_render_offset", 0.125);
	bgKV->SetInt("justify_horizontal", 0); // 0代表左对齐
	bgKV->SetInt("justify_vertical", 1);   // 1代表垂直居中
	bgKV->SetInt("reorient_mode", 0);      // don't change
	bgKV->SetInt("fullbright", 1);
	bgKV->SetFloat("font_size", bgFontSize);

	Vector bgPos = GetAimPoint(vmPos, vmAng, 7.29f);
	QAngle bgAng = vmAng;
	bgAng.y -= 90.0f;
	bgAng.z += 90.0f;
	bgKV->SetQAngle("angles", bgAng);

	AngleVectors(bgAng, &rig, &dwn, nullptr);

	rig *= (g_fMenuOffsetX - 0.2f);
	dwn *= (g_fMenuOffsetY + 2.0f);

	bgPos += rig + dwn;
	bgKV->SetVector("origin", bgPos);

	bgEntity->DispatchSpawn(bgKV);
	fnSetParent(bgEntity, pViewModel);
	bgEntity->m_hOwnerEntity(pViewModel->GetRefEHandle());

	bgEntity->SetText("█");
}

class TestMenu : CCoreForward {
private:
	virtual void OnPluginStart() {
		CONCMD::RegConsoleCmd("sm_mtest", MENU_TEST);
	}
};

TestMenu g_TestMenu;
