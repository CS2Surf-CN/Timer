#include <pch.h>

constexpr auto m_vecVelocity_maxValue = 10000.0f;

class CMaxVelocityFix : CCoreForward {
private:
	virtual void OnPluginStart() override;
};

CMaxVelocityFix g_MaxVelocityFix;

void CMaxVelocityFix::OnPluginStart() {
	float* min = (float*)GAMEDATA::GetAddress("m_vecVelocity_min");
	libmem::ProtMemory((libmem::Address)min, sizeof(float), libmem::Prot::XRW);
	*min = -m_vecVelocity_maxValue;

	float* max = (float*)GAMEDATA::GetAddress("m_vecVelocity_max");
	libmem::ProtMemory((libmem::Address)max, sizeof(float), libmem::Prot::XRW);
	*max = m_vecVelocity_maxValue;
}
