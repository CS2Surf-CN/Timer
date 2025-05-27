#include <core/memory.h>

class CMovementUnlocker : CCoreForward {
private:
	virtual void OnPluginStart() override;
};

CMovementUnlocker g_MovementUnlocker;

void CMovementUnlocker::OnPluginStart() {
	static auto fn = GAMEDATA::GetMemSig("ServerMovementUnlock");
	SDK_ASSERT(fn);

	WIN_LINUX(MEM::PatchAddress(fn, 0xEB), MEM::PatchAddress(fn, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90));
}
