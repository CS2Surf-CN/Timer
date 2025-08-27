#pragma once

#include <surf/surf_player.h>
#include <surf/zones/edit.h>
#include <hv/HttpMessage.h>

#ifndef HTTPRES_CALLBACK
#define HTTPRES_CALLBACK(fn) static void fn(const HttpResponsePtr& res)
#endif

#ifndef HTTPRES_CALLBACK_L
#define HTTPRES_CALLBACK_L(...) [__VA_ARGS__](const HttpResponsePtr& res) -> void
#endif

#define GAPIRES_CHECK(http_res, gapi_res, ...) \
	if (!http_res || http_res->status_code != HTTP_STATUS_OK) { \
		do { \
			__VA_ARGS__; \
		} while (0); \
		return; \
	} \
	GlobalAPIResponse gapi_res(http_res->body); \
	if (gapi_res.m_iCode != HTTP_STATUS_OK) { \
		do { \
			__VA_ARGS__; \
		} while (0); \
		return; \
	}

struct GlobalAPIRequest : json {
	int m_iTimeout = 10;
	std::string m_sCustomToken = "";
	http_method m_iMethod = HTTP_GET;
};

struct GlobalAPIResponse {
	int m_iCode = -1;
	std::string m_sMessage;
	json m_Data;
	uint64 m_iTimestamp = -1;

	GlobalAPIResponse() = default;

	GlobalAPIResponse(const std::string& body) {
		if (!body.empty()) {
			json j = json::parse(body);
			FromJson(j);
		}
	}

	GlobalAPIResponse(const json& body) {
		FromJson(body);
	}

	json ToJson() const;
	void FromJson(const json& j);
};

class CSurfGlobalAPIPlugin : CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnClientActive(ISource2GameClients* pClient, CPlayerSlot slot, bool bLoadGame, const char* pszName, uint64 xuid) override;
	virtual void OnApplyGameSettings(ISource2Server* pServer, KeyValues* pKV) override;

private:
	void Reset();
	void RegisterCommand();

public:
	void ReadAPIKey();
	bool MapCheck() const;
	void GlobalCheck(CBasePlayerController* pController) const;
	bool IsGlobalEnabled() const;
	bool IsGlobalUpdaterEnabled() const;

public:
	void CreateRequest(std::string sEndpointAlias, const GlobalAPIRequest& req, HttpResponseCallback& cb);

public:
	bool m_bBannedCommandsCheck;
	bool m_bEnforcerOnFreshMap;
	bool m_bMapValidated;
	uint64 m_iMapWorkshopID;
	std::string m_sMapName;
	std::unordered_map<std::string, std::string> m_umEndpoint;

public:
	struct GlobalAPIPayload {
		std::string m_sKey;
		std::string m_sToken;
	};

	GlobalAPIPayload m_GlobalAuth;
	GlobalAPIPayload m_UpdaterAuth;
};

class CSurfGlobalAPIService : CSurfPlayerService {
private:
	virtual void OnPlayerQuit() override;

public:
	using CSurfPlayerService::CSurfPlayerService;

	void CheckGlobalBan();

public:
	bool m_bGloballyVerified;
};

namespace SURF {
	extern CSurfGlobalAPIPlugin* GlobalPlugin();

	namespace GLOBALAPI {
		inline constexpr const char* BaseUrl = "http://localhost:3000";

		namespace AUTH {
			void GetGlobalToken(std::string global_key, HttpResponseCallback cb);
			void GetUpdaterToken(std::string global_key, HttpResponseCallback cb);
		} // namespace AUTH

		namespace BAN {}

		namespace MAP {
			inline struct mapinfo_t {
				int m_iTier = 0;
				float m_fMaxvel = 3500.0f;
				bool m_bLimitPrespeed = false;
			} g_MapInfo;

			void UpdateInfo(uint64 workshopID, std::string mapName, const mapinfo_t& info, HttpResponseCallback cb);
			void PullInfo(uint64 workshopID, std::string mapName, HttpResponseCallback cb);

			struct zoneinfo_t : ZoneData_t {
				zoneinfo_t();
				zoneinfo_t(const ZoneData_t& other);

				std::string m_sMap;

				json ToJson() const;
				void FromJson(const nlohmann::json& j);
			};

			void UpdateZone(const zoneinfo_t& info, HttpResponseCallback cb);
			void DeleteZone(const zoneinfo_t& info, HttpResponseCallback cb);
			void DeleteAllZones(HttpResponseCallback cb);
			void PullZone(HttpResponseCallback cb);
		} // namespace MAP

		namespace STYLE {}

		namespace PLAYER {}

		namespace RECORD {}
	} // namespace GLOBALAPI
} // namespace SURF
