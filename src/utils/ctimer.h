#pragma once

#include <functional>
#include <tuple>

#include <sdk/common.h>
#include <sdk/datatypes.h>

/*
 * Credit to cs2kz
 */

struct TimerOption_t {
	bool useRealTime = false;
	bool preserveMapChange = false;
};

class CTimerBase {
public:
	CTimerBase(f64 initialInterval, bool useRealTime)
		: interval(initialInterval), useRealTime(useRealTime) {};

	virtual ~CTimerBase() {}

	virtual bool Execute() = 0;

	f64 interval {};
	f64 lastExecute = -1;
	bool removeable {};
	bool useRealTime {};
};

template<typename... Args>
class CTimer : public CTimerBase {
public:
	using Fn = std::function<f64(Args...)>;

	Fn m_fn;
	std::tuple<Args...> m_args;

	explicit CTimer(bool useRealTime, f32 initialDelay, Fn fn, Args... args)
		: CTimerBase(initialDelay, useRealTime), m_fn(std::move(fn)), m_args(std::make_tuple(std::move(args)...)) {}

	bool Execute() override {
		interval = std::apply(m_fn, m_args);
		return interval > 0;
	}
};

class CTimerHandle : public CStdWeakHandle<CTimerBase> {
public:
	using CStdWeakHandle::CStdWeakHandle;

	bool Close();
};

class IFrameAction {
public:
	virtual ~IFrameAction() {}

	virtual void Execute() = 0;
};

template<typename... Args>
class CFrameAction : public IFrameAction {
public:
	using Fn = std::function<void(Args...)>;

	Fn m_fn;
	std::tuple<Args...> m_args;

	explicit CFrameAction(Fn fn, Args... args)
		: m_fn(std::move(fn)), m_args(std::make_tuple(std::move(args)...)) {}

	virtual void Execute() override {
		std::apply(m_fn, m_args);
	}
};

namespace UTIL {
	namespace TIMER {
		void AddTimer(const std::shared_ptr<CTimerBase>& pTimer, bool preserveMapChange = true);
		void RemoveTimer(CTimerHandle& hTimer);
		void AddFrameAction(std::unique_ptr<IFrameAction> action);
	} // namespace TIMER

	// Creates a timer for the given function, the function must return a f64 that represents the interval in seconds; 0 or less to stop the timer
	template<typename... Args>
	CTimerHandle CreateTimer(f64 initialDelay, typename CTimer<Args...>::Fn fn, Args... args) {
		auto pTimer = std::make_shared<CTimer<std::decay_t<Args>...>>(false, initialDelay, fn, args...);
		TIMER::AddTimer(pTimer, false);
		return CTimerHandle(std::static_pointer_cast<CTimerBase>(pTimer));
	}

	// See CreateTimer.
	// With options.
	template<typename... Args>
	CTimerHandle CreateTimerEx(f64 initialDelay, TimerOption_t opt, typename CTimer<Args...>::Fn fn, Args... args) {
		auto pTimer = std::make_shared<CTimer<std::decay_t<Args>...>>(opt.useRealTime, initialDelay, fn, args...);
		TIMER::AddTimer(pTimer, opt.preserveMapChange);
		return CTimerHandle(std::static_pointer_cast<CTimerBase>(pTimer));
	}

	// Request a frame action for the given function
	template<typename... Args>
	void RequestFrame(typename CFrameAction<Args...>::Fn fn, Args... args) {
		auto pFrameAction = std::make_unique<CFrameAction<std::decay_t<Args>...>>(fn, args...);
		TIMER::AddFrameAction(std::move(pFrameAction));
	}
} // namespace UTIL
