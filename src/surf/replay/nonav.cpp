#include <pch.h>
#include <core/memory.h>

class CNoNavPlugin : CCoreForward {
	virtual void OnPluginStart() override;
};

CNoNavPlugin g_NoNav;

void CNoNavPlugin::OnPluginStart() {
	static auto fn = GAMEDATA::GetMemSig("BotNavIgnore");
	SDK_ASSERT(fn);

	MEM::PatchNOP(fn, WIN_LINUX(13, 36));
}
