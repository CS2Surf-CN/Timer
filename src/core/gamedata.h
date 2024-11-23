#pragma once

#include <string>
#include <unordered_map>

#include <sdk/common.h>

// for one gamedata cfg now
namespace GAMEDATA {
	void Append(std::string sFilePath);
	int GetOffset(std::string name);
	void* GetMemSig(std::string name);
	void* GetAddress(std::string name);

	inline json m_Json;
	inline std::string m_sFilePath;
	inline std::unordered_map<std::string, void*> m_pMemSig;
}; // namespace GAMEDATA