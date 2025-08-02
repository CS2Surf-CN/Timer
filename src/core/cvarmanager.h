#pragma once

#include <tier1/convar.h>
#include <optional>

extern void ReplicateConVarValue(const char* name, const char* value, CPlayerSlot slot);

namespace CVAR {
	// clang-format off
	
	// Must be on OnPluginStart
	template<typename T, typename TMin = std::optional<T>, typename TMax = std::optional<T>>
	ConVarRefAbstract* Register(const char* name, T defaultValue, const char* helpString,
								TMin minValue = std::nullopt,
								TMax maxValue = std::nullopt,
								typename FnTypedChangeCallback_t<T> callback = nullptr) {
		auto hasValue = [](auto& v) -> bool {
			using RawType = std::decay_t<decltype(v)>;
			return std::is_same_v<RawType, T>;
		};

		auto getValue = [](auto& v) -> T {
			using RawType = std::decay_t<decltype(v)>;
			if constexpr (std::is_same_v<RawType, T>) {
				return v;
			} else {
				return T{};
			}
		};

		CConVar<T>* pCvarCreated = new CConVar<T>(name, 0, helpString, defaultValue, 
			hasValue(minValue), getValue(minValue), 
			hasValue(maxValue), getValue(maxValue), 
			callback);
		return pCvarCreated;
	}

	// clang-format on

	std::optional<ConVarRefAbstract> Find(const char* name);

	template<typename T>
	bool Set(const char* name, T value) {
		auto targetCvar = Find(name);
		if (!targetCvar) {
			return false;
		}

		auto& targetCvarRef = targetCvar.value();
		targetCvarRef.SetAs(value);

		return true;
	}

	template<typename T>
	bool Replicate(const char* name, T value, CPlayerSlot slot) {
		auto targetCvar = Find(name);
		if (!targetCvar) {
			return false;
		}

		auto& targetCvarRef = targetCvar.value();

		CBufferString buffer;
		targetCvarRef.TypeTraits()->ValueToString(value, buffer);
		ReplicateConVarValue(name, buffer.Get(), slot);

		return true;
	}
} // namespace CVAR
