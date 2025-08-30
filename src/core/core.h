#pragma once

#include <json.h>
using json = nlohmann::json;

#include <sdk/entity/ccsplayercontroller.h>

#define SCMD_CALLBACK_ARGS   const std::vector<std::string>& vArgs
#define SCMD_CALLBACK(fn)    static void fn(SCMD_CALLBACK_ARGS)
#define SCMD_CALLBACK_L(...) [__VA_ARGS__](SCMD_CALLBACK_ARGS) -> void
using SrvCmd_Callback = std::function<void(SCMD_CALLBACK_ARGS)>;

#define CCMD_CALLBACK_ARGS   CCSPlayerController* pController, const std::vector<std::string>& vArgs, const std::wstring& wCommand
#define CCMD_CALLBACK(fn)    static void fn(CCMD_CALLBACK_ARGS)
#define CCMD_CALLBACK_L(...) [__VA_ARGS__](CCMD_CALLBACK_ARGS) -> void
using ConCmd_Callback = std::function<void(CCMD_CALLBACK_ARGS)>;

#define CCMDLISTENER_CALLBACK(fn)    static bool fn(CCMD_CALLBACK_ARGS)
#define CCMDLISTENER_CALLBACK_L(...) [__VA_ARGS__](CCMD_CALLBACK_ARGS) -> bool
using ConCmdListener_Callback = std::function<bool(CCMD_CALLBACK_ARGS)>;

#include <sdk/forwardbase.h>

#define FORWARD_INIT(forwardClass) \
	template<> \
	forwardClass* CBaseForward<forwardClass>::m_pFirst = nullptr;

#define FORWARD_PRE_void(forwardClass, forwardFn, ...) \
	for (auto p = forwardClass::m_pFirst; p; p = p->m_pNext) { \
		if (!p->forwardFn(__VA_ARGS__)) { \
			return; \
		} \
	}

#define FORWARD_PRE(forwardClass, forwardFn, ret, ...) \
	for (auto p = forwardClass::m_pFirst; p; p = p->m_pNext) { \
		if (!p->forwardFn(__VA_ARGS__)) { \
			return ret; \
		} \
	}

#define FORWARD_POST(forwardClass, forwardFn, ...) \
	for (auto p = forwardClass::m_pFirst; p; p = p->m_pNext) { \
		p->forwardFn(__VA_ARGS__); \
	}
