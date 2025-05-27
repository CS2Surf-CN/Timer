#pragma once

#include <sstream>
#include <cstring>
#include <unordered_map>

#ifdef _WIN32
#define MODULE_PREFIX ""
#define MODULE_EXT    ".dll"
#else
#define MODULE_PREFIX "lib"
#define MODULE_EXT    ".so"
#endif

namespace libmem {
	inline Module GetModule(const char* name) {
		static std::unordered_map<std::string, Module> mods;
		auto it = mods.find(name);
		if (it != mods.end()) {
			return it->second;
		}

		Module mod;
		std::vector<libmem::Module> modules;
		auto value = EnumModules();
		if (value) {
			modules = value.value();
		}

		// dont get our own server.dll module.
		constexpr const char* libServer = MODULE_PREFIX "server" MODULE_EXT;
		bool is_find_server_hdl = !strcmp(name, libServer);

		for (auto& it : modules) {
			if (is_find_server_hdl && strstr(it.path.c_str(), "addons") != nullptr) {
				continue;
			}

			if (!strcmp(it.name.c_str(), name)) {
				mod = Module(it);
				break;
			}
		}

		mods[name] = mod;
		return mod;
	}

	inline Address FindSymbolAddress(const Module& module, const char* symbol_name) {
		auto res = FindSymbolAddress(&module, symbol_name);
		return res.has_value() ? res.value() : (Address) nullptr;
	}

	inline Address FindSymbolAddress(const char* module, const char* name) {
		auto pModule = GetModule(module);
		if (!pModule.base) {
			return (Address) nullptr;
		}

		return FindSymbolAddress(&pModule, name).value();
	}

	inline auto PatternScan(const unsigned char* pattern, const char* mask, const Module& module) {
		std::vector<uint8_t> vecPattern;
		std::istringstream iss(reinterpret_cast<const char*>(pattern));

		uint8_t val;
		while (iss >> std::hex >> val) {
			vecPattern.push_back(val);
		}

		PatternScan(vecPattern, mask, module.base, module.size);
	}

	inline auto PatternScan(const unsigned char* pattern, const char* mask, const char* moduleName) {
		return PatternScan(pattern, mask, GetModule(moduleName));
	}

	inline void* SignScan(const char* sig, const char* moduleName) {
		auto module = GetModule(moduleName);
		auto val = SigScan(sig, module.base, module.size);
		return val.has_value() ? (void*)val.value() : nullptr;
	}

	inline void* SignScan(const char* sig, const Module& module) {
		auto val = SigScan(sig, module.base, module.size);
		return val.has_value() ? (void*)val.value() : nullptr;
	}
} // namespace libmem
