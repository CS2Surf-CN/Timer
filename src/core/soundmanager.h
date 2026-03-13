#pragma once

using SoundEventGuid_t = uint;
constexpr auto INVALID_SND_GUID = ~0u;

using SoundEventName_t = CUtlStringToken;
constexpr auto INVALID_SND_NAME = ~0u;

#pragma pack(push, 1)

template<typename T>
struct SoundEventParams_t {
	SoundEventParams_t(const std::string_view svSoundEvent, uint8 type)
		: m_nName(CUtlStringToken(svSoundEvent.data())), m_nType(type), m_nFieldDataSize(sizeof(T)), m_nAllocSize(0), m_Value(T {}) {}

	std::string Serialize() {
		std::string result;
		result.resize(sizeof(SoundEventParams_t<T>));

		std::memcpy(result.data(), this, sizeof(SoundEventParams_t<T>));

		return result;
	}

	SoundEventName_t m_nName;
	uint8 m_nType;
	uint8 m_nFieldDataSize;
	uint8 m_nAllocSize;
	T m_Value;
};

#pragma pack(pop)

struct SndOpEventGuid_t {
	SoundEventGuid_t m_nGuid = 0;
	SoundEventName_t m_nName = INVALID_SND_NAME;
};

static_assert(sizeof(SndOpEventGuid_t) == 8);

struct StartSoundEventInfo_t {
	SndOpEventGuid_t m_nSndOpEventGuid = {};

	enum EventFlag : uint32 {
		DEFAULT = (1 << 0),
		RELIABLE = (1 << 1),
		INIT = (1 << 2),
		VOICE = (1 << 3)
	} m_nFlags = {};

	CPlayerBitVec m_Players;
};

static_assert(sizeof(StartSoundEventInfo_t) == 20);

struct EmitSound_t {
	const char* m_pSoundName = {};
	Vector m_veSoundOrigin = {};
	float m_flVolume = 1.f;
	float m_flSoundTime = {};
	CEntityIndex m_nSpeakerEntity = -1;
	SndOpEventGuid_t m_nForceGuid {};
	uint16 m_nPitch = 100;

	struct {
		bool m_bEmitCloseCaption: 1 = true;
		bool m_bWarnOnMissingCloseCaption: 1 = false;
		bool m_bWarnOnDirectWaveReference: 1 = false;
		bool m_bUseSpeakerEntity: 1 = false;
		bool m_bEmitAtPosition: 1 = true;
		uint8 m_nReserved: 3 = 0u;
	} m_nFlags;
};

static_assert(offsetof(EmitSound_t, m_nFlags) == 42);

namespace SOUND {
	StartSoundEventInfo_t EmitSoundFilter(const IRecipientFilter& filter, const std::string_view svSoundName, float flVolume = 1.0f, float flPitch = 1.0f, CBaseEntity* pEntity = nullptr);
	void SetVolume(const IRecipientFilter& filter, float flVolume, SoundEventGuid_t guid);
	void SetVolume(CNetMessage* pSetSoundEventParamsMsg, float flVolume, SoundEventGuid_t guid = INVALID_SND_GUID);
} // namespace SOUND
