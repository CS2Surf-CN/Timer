#include "ctimer.h"
#include <queue>
#include <list>

#include <utils/utils.h>
#include <core/forwards.h>

std::list<std::shared_ptr<CTimerBase>> g_NonPersistentTimers;
std::list<std::shared_ptr<CTimerBase>> g_PersistentTimers;

std::mutex g_FrameMutex;
std::queue<std::unique_ptr<IFrameAction>> g_FrameQueue;
std::queue<std::unique_ptr<IFrameAction>> g_FrameActions;

static void ProcessFrameActions() {
	if (g_FrameQueue.size()) {
		g_FrameMutex.lock();
		std::swap(g_FrameQueue, g_FrameActions);
		g_FrameMutex.unlock();

		while (!g_FrameActions.empty()) {
			g_FrameActions.front()->Execute();
			g_FrameActions.pop();
		}
	}
}

static void ProcessTimerList(std::list<std::shared_ptr<CTimerBase>>& timers) {
	for (auto it = timers.begin(); it != timers.end();) {
		auto& pTimer = *it;
		f64 currentTime = pTimer->useRealTime ? UTIL::GetGlobals()->realtime : UTIL::GetGlobals()->curtime;
		if (pTimer->lastExecute == -1) {
			pTimer->lastExecute = currentTime;
		}

		if (pTimer->lastExecute + pTimer->interval <= currentTime) {
			if (pTimer->removeable || !pTimer->Execute()) {
				it = timers.erase(it);
				continue;
			} else {
				pTimer->lastExecute = currentTime;
			}
		}

		++it;
	}
}

static void RemoveNonPersistentTimers() {
	g_NonPersistentTimers.clear();
}

void UTIL::TIMER::AddTimer(const std::shared_ptr<CTimerBase>& timer, bool preserveMapChange) {
	if (preserveMapChange) {
		g_PersistentTimers.emplace_back(timer);
	} else {
		g_NonPersistentTimers.emplace_back(timer);
	}
}

void UTIL::TIMER::RemoveTimer(CTimerHandle& hTimer) {
	if (!hTimer) {
		return;
	}

	auto RemoveFromTimer = [pTimer = hTimer.Data()](std::list<std::shared_ptr<CTimerBase>>& timers) {
		auto it = std::find_if(timers.begin(), timers.end(), [pTimer](const auto& ptr) {
			return ptr.get() == pTimer;
		});

		if (it != timers.end()) {
			(*it)->removeable = true;
		}
	};

	RemoveFromTimer(g_PersistentTimers);
	RemoveFromTimer(g_NonPersistentTimers);
}

void UTIL::TIMER::AddFrameAction(std::unique_ptr<IFrameAction> action) {
	g_FrameMutex.lock();
	g_FrameQueue.push(std::move(action));
	g_FrameMutex.unlock();
}

class CProcessTimer : CCoreForward {
private:
	virtual void OnServerGamePostSimulate(IGameSystem* pGameEvent) override {
		::ProcessFrameActions();
		::ProcessTimerList(g_PersistentTimers);
		::ProcessTimerList(g_NonPersistentTimers);
	}
};

CProcessTimer g_ProcessTimer;

bool CTimerHandle::Close() {
	if (!IsValid()) {
		Reset();
		return false;
	}

	UTIL::TIMER::RemoveTimer(*this);

	return true;
}
