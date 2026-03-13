#include "pch.h"
#include "memory.h"
#include "interfaces.h"
#include "soundmanager.h"
#include <gameevents.pb.h>

StartSoundEventInfo_t SOUND::EmitSoundFilter(const IRecipientFilter& filter, const std::string_view svSoundName, float flVolume, float flPitch, CBaseEntity* pEntity) {
	EmitSound_t params;
	params.m_pSoundName = svSoundName.data();
	params.m_flVolume = flVolume;
	params.m_nPitch = static_cast<uint16>(flPitch * 100);

	StartSoundEventInfo_t info;

	MEM::CALL::EmitSoundByHandle(&info, &filter, pEntity ? pEntity->GetEntityIndex() : 0, &params);

	SetVolume(filter, flVolume, info.m_nSndOpEventGuid.m_nGuid);

	return info;
}

void SOUND::SetVolume(const IRecipientFilter& filter, float flVolume, SoundEventGuid_t guid) {
	INetworkMessageInternal* netmsg = g_pNetworkMessages->FindNetworkMessagePartial("SosSetSoundEventParams");
	auto msg = netmsg->AllocateMessage()->ToPB<CMsgSosSetSoundEventParams>();
	SetVolume(msg, flVolume, guid);

	IFACE::pGameEventSystem->PostEventAbstract(0, false, const_cast<IRecipientFilter*>(&filter), netmsg, msg, 0);
}

void SOUND::SetVolume(CNetMessage* pSetSoundEventParamsMsg, float flVolume, SoundEventGuid_t guid) {
	auto pMsg = reinterpret_cast<CNetMessagePB<CMsgSosSetSoundEventParams>*>(pSetSoundEventParamsMsg);

	static SoundEventParams_t<float> param("public.volume", 8);
	param.m_Value = flVolume;
	pMsg->set_packed_params(param.Serialize());

	if (guid != INVALID_SND_GUID) {
		pMsg->set_soundevent_guid(guid);
	}
}
