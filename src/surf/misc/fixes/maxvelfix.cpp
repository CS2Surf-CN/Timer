#include <pch.h>

constexpr auto m_vecVelocity_maxValue = 10000.0f;

class CMaxVelocityFix : CCoreForward {
private:
	virtual void OnPluginStart() override;
};

CMaxVelocityFix g_MaxVelocityFix;

void CMaxVelocityFix::OnPluginStart() {
	{
		float* pFlValue = (float*)GAMEDATA::GetAddress("m_vecVelocity_min");
		if (auto res = safetyhook::unprotect(reinterpret_cast<uint8_t*>(pFlValue), sizeof(float)); res) {
			safetyhook::store(reinterpret_cast<uint8_t*>(pFlValue), -m_vecVelocity_maxValue);
		}
	}
	{
		float* pFlValue = (float*)GAMEDATA::GetAddress("m_vecVelocity_max");
		if (auto res = safetyhook::unprotect(reinterpret_cast<uint8_t*>(pFlValue), sizeof(float)); res) {
			safetyhook::store(reinterpret_cast<uint8_t*>(pFlValue), m_vecVelocity_maxValue);
		}
	}
}
