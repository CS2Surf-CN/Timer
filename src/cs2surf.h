#pragma once

#include <ISmmPlugin.h>

class CSurfPlugin : public ISmmPlugin, public IMetamodListener {
private:
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) override;
	void AllPluginsLoaded() override;
	const char* GetAuthor() override;
	const char* GetName() override;
	const char* GetDescription() override;
	const char* GetURL() override;
	const char* GetLicense() override;
	const char* GetVersion() override;
	const char* GetDate() override;
	const char* GetLogTag() override;

public:
	void AddonInit();
	bool IsAddonMounted();

public:
	bool simulatingPhysics = false;
	CGlobalVars serverGlobals;
};

extern CSurfPlugin* SurfPlugin();
