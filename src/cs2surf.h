#pragma once

#include <ISmmPlugin.h>

class CSurfPlugin : public ISmmPlugin, public IMetamodListener {
private:
	virtual bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) override;
	virtual void AllPluginsLoaded() override;
	virtual void OnLevelInit(char const* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background) override;
	virtual const char* GetAuthor() override;
	virtual const char* GetName() override;
	virtual const char* GetDescription() override;
	virtual const char* GetURL() override;
	virtual const char* GetLicense() override;
	virtual const char* GetVersion() override;
	virtual const char* GetDate() override;
	virtual const char* GetLogTag() override;

public:
	void AddonInit();
	bool IsAddonMounted();

public:
	bool simulatingPhysics = false;
	CGlobalVars serverGlobals;
	std::string m_sCurrentMap;
};

extern CSurfPlugin* SurfPlugin();
