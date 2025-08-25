#include "recipientfilters.h"
#include <utils/utils.h>

void CRecipientFilter::AddAllPlayers() {
	m_Recipients.ClearAll();

	auto pClientList = UTIL::GetClientList();

	for (int i = 0; i < pClientList->Count(); i++) {
		auto pClient = pClientList->Element(i);
		if (pClient->IsInGame()) {
			AddRecipient(pClient->GetPlayerSlot());
		}
	}
}
