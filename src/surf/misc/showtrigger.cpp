#include "showtrigger.h"
#include <libmem/libmem_helper.h>
#include <core/memory.h>

CShowTrigger g_ShowTrigger;

enum S2EdictState {
	FL_EDICT_FULL_S2 = 0x1,
	FL_FULL_EDICT_CHANGED_S2 = 0x2,
	FL_EDICT_ALWAYS_S2 = 0x4,
	FL_EDICT_DONTSEND_S2 = 0x8,
	FL_EDICT_PVSCHECK_S2 = 0x10,
};

void CShowTrigger::TransmitTriggers(const std::vector<CHandle<CBaseTrigger>>& vTriggers, bool bTransmit) {
	for (const auto& hTrigger : vTriggers) {
		CBaseTrigger* pTrigger = hTrigger.Get();
		if (!pTrigger) {
			continue;
		}

		int iEffect = pTrigger->m_fEffects();
		int iStateFlags = pTrigger->m_NetworkTransmitComponent()->m_iStateFlags();

		auto fnGetEffect = libmem::SignScan("48 8B D1 48 8B 0D A6 07 2D 01", LIB::server);
		auto test = MEM::SDKCall<int64_t>(fnGetEffect, pTrigger);

		auto fnRmEffect = libmem::SignScan("4C 8B C2 48 8B D1 48 8B 0D 93 66 2D 01", LIB::server);
		auto fnAddEffect = libmem::SignScan("4C 8B C2 48 8B D1 48 8B 0D D3 66 2C 01", LIB::server);
		//MEM::SDKCall<void*>(fnAddEffect, pTrigger, EF_NOINTERP);

		if ((test & 0x2000) != 0) {
			MEM::SDKCall<void*>(fnRmEffect, pTrigger, 0x2000);
		} else {
			MEM::SDKCall<void*>(fnAddEffect, pTrigger, 0x2000);
		}

		if (bTransmit) {
			iEffect &= ~(EF_NODRAW);
			iStateFlags &= ~FL_EDICT_DONTSEND_S2;
			//iStateFlags &= ~FL_FULL_EDICT_CHANGED_S2;
			//iStateFlags |= (FL_EDICT_PVSCHECK_S2 | FL_EDICT_FULL_S2);
		} else {
			iEffect |= (EF_NODRAW);
			iStateFlags |= FL_EDICT_DONTSEND_S2;
			//iStateFlags &= ~FL_EDICT_PVSCHECK_S2;
		}

		//pTrigger->m_fEffects(iEffect);

		// auto target = (uint*)((char*)pTrigger + 0x1c0);
		//_InterlockedExchange(target, iStateFlags);
		pTrigger->m_NetworkTransmitComponent()->m_iStateFlags(iStateFlags);
		pTrigger->NetworkStateChanged();
	}
}
