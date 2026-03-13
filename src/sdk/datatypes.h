#pragma once

#include <gametrace.h>

class CBasePlayerPawn;

struct touchlist_t {
	Vector deltavelocity;
	trace_t trace;
};

class CTraceFilterPlayerMovementCS : public CTraceFilter {
public:
	CTraceFilterPlayerMovementCS(CBasePlayerPawn* pawn);

	virtual bool ShouldHitEntity(CEntityInstance* pEnt) override;
};

template<typename T>
class CStdWeakHandle {
public:
	CStdWeakHandle() = default;

	CStdWeakHandle(const std::shared_ptr<T>& pData)
		: m_wpData(pData) {}

	operator bool() const {
		return IsValid();
	}

	T* Data() {
		return m_wpData.lock().get();
	}

	bool IsValid() const {
		return !this->m_wpData.expired();
	}

	void Reset() {
		this->m_wpData.reset();
	}

private:
	std::weak_ptr<T> m_wpData;
};
