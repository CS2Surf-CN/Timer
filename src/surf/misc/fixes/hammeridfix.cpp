#include <pch.h>

class CHammeridFix : CCoreForward {
private:
	virtual void OnPluginStart() override;
};

CHammeridFix g_HammeridFix;

// https://github.com/Source2ZE/CS2Fixes/blob/f52b5566c750dbe7019051dbd1bddf4c361ff2c6/src/entitylistener.cpp#L35
void CHammeridFix::OnPluginStart() {
	auto pVtable = MEM::MODULE::server->GetVirtualTableByName("CBaseEntity").RCast<void**>();
	if (!pVtable) {
		SDK_ASSERT(false);
		return;
	}

	auto iOffset = GAMEDATA::GetOffset("CBaseEntity::GetHammerUniqueId");
	MEM::PatchAddress(pVtable[iOffset], 0xB0, 0x01);
}
