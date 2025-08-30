#pragma once

#include <pch.h>

#include <string>
#include <filesystem>

#include <sdk/serversideclient.h>
#include <sdk/gamerules.h>

#include <utils/ctimer.h>
#include <utils/print.h>

import surf.core;

#define JSON_GETTER(_json, key, ret) \
	do { \
		if (_json.contains(#key)) \
			_json.at(#key).get_to(ret); \
	} while (0)

namespace UTIL {
	namespace PATH {
		template<typename... Args>
		std::string Join(const Args&... args) {
			std::filesystem::path result;
			((result /= std::filesystem::path(args)), ...);
			return result.string();
		}
	} // namespace PATH

	namespace VECTOR {
		template<typename T>
		std::vector<T> Slice(const std::vector<T>& v, size_t start, size_t end) {
			if (start >= end || start >= v.size()) {
				return {};
			}
			end = std::min(end, v.size());
			return std::vector<T>(v.begin() + start, v.begin() + end);
		}
	} // namespace VECTOR

	namespace PB {
		template<typename T>
		bool ReadFromBuffer(bf_read& buffer, T& pb) {
			auto size = buffer.ReadVarInt32();

			if (size < 0 || size > 262140) {
				return false;
			}

			if (size > buffer.GetNumBytesLeft()) {
				return false;
			}

			if ((buffer.GetNumBitsRead() % 8) == 0) {
				bool parseResult = pb.ParseFromArray(buffer.GetBasePointer() + buffer.GetNumBytesRead(), size);
				buffer.SeekRelative(size * 8);

				return true;
			}

			void* parseBuffer = stackalloc(size);
			if (!buffer.ReadBytes(parseBuffer, size)) {
				return false;
			}

			if (!pb.ParseFromArray(parseBuffer, size)) {
				return false;
			}

			return true;
		}
	} // namespace PB

	std::string GetWorkingDirectory();
	std::string GetPublicIP();
	json LoadJsonc(std::string sFilePath);
	std::wstring ToWideString(const char* pszCharStr);
	void ReplaceString(std::string& str, const char* from, const char* to);

	CGlobalVars* GetGlobals();
	CGlobalVars* GetServerGlobals();
	CBasePlayerController* GetController(CBaseEntity* entity);
	CBasePlayerController* GetController(CPlayerSlot slot);
	bool IsPlayerSlot(CPlayerSlot slot);
	CUtlVector<CServerSideClient*>* GetClientList();
	CServerSideClient* GetClientBySlot(CPlayerSlot slot);
	CCSGameRules* GetGameRules();
	std::string GetCurrentMap();

	bool TraceLine(const Vector& vecStart, const Vector& vecEnd, CEntityInstance* ignore1, CGameTrace* tr, uint64 traceLayer, uint64 excludeLayer = 0);
	void GetPlayerAiming(CCSPlayerPawnBase* pPlayer, CGameTrace& ret);
	void PlaySoundToClient(CPlayerSlot player, const char* sound, f32 volume = 1.0f);

	CBaseEntity* FindEntityByClassname(CEntityInstance* start, const char* name);
	CBaseEntity* CreateBeam(const Vector& from, const Vector& to, Color color = Color(0, 255, 0, 255), float width = 1.5f, CBaseEntity* owner = nullptr);

#pragma region ConVar
	void UnlockConVars();
	void UnlockConCommands();
	void SendConVarValue(CPlayerSlot slot, const char* cvar, const char* value);
	void SendConVarValue(CPlayerSlot slot, ConVarRefAbstract* cvar, const char* value);
	void SendMultipleConVarValues(CPlayerSlot slot, const char** cvars, const char** values, u32 size);
	void SendMultipleConVarValues(CPlayerSlot slot, ConVarRefAbstract** cvars, const char** values, u32 size);
#pragma endregion
} // namespace UTIL
