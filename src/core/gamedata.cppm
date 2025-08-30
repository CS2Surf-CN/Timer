module;

#include <string>

export module surf.core.gamedata;

// for one gamedata cfg now
export namespace GAMEDATA {
	void Append(std::string sFilePath);
	int GetOffset(std::string name);
	void* GetMemSig(std::string name);
	void* GetAddress(std::string name);
}; // namespace GAMEDATA

