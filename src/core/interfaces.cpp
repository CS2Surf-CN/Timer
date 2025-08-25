#include "interfaces.h"
#include "gamedata.h"

#include <playerslot.h>
#include <vector.h>
#include <igameeventsystem.h>

bool IFACE::Setup(ISmmAPI* ismm, char* error, size_t maxlen) {
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, IFACE::pGameResourceServiceServer, CGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2GameClients, ISource2GameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2GameEntities, ISource2GameEntities, SOURCE2GAMEENTITIES_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, IFACE::pEngine, IVEngineServer2, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetServerFactory, IFACE::pServer, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, IFACE::pGameEventSystem, IGameEventSystem, GAMEEVENTSYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	return true;
}

bool IFACE::PostSetup() {
	IFACE::pGameEventManager = (IGameEventManager2*)GAMEDATA::GetAddress("GetGameEventManager");
	IFACE::pEngineTrace = (CGameTraceService*)GAMEDATA::GetAddress("g_pEngineTrace");

	return true;
}
