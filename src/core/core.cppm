module;
#include <json.h>

export module surf.core;

export import surf.core.adminmanager;
export import surf.core.gamedata;

export using json = nlohmann::json;
