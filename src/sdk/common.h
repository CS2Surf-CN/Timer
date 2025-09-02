#pragma once

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define CS_TEAM_NONE      0
#define CS_TEAM_SPECTATOR 1
#define CS_TEAM_T         2
#define CS_TEAM_CT        3

#define ENGINE_FIXED_TICK_INTERVAL 0.015625f
#define ENGINE_FIXED_TICK_RATE     (1.0f / ENGINE_FIXED_TICK_INTERVAL)
#define EPSILON                    0.000001f

#ifndef SDK_DEBUG_BREAK
#ifdef _WIN32
#define SDK_DEBUG_BREAK() __debugbreak()
#else
#define SDK_DEBUG_BREAK() __builtin_trap()
#endif
#endif

#ifndef SDK_DEBUG
#if defined(_DEBUG) || defined(_OD)
#define SDK_DEBUG
#endif
#endif

#ifndef SDK_ASSERT
#ifdef SDK_DEBUG
#define SDK_ASSERT(EXPRESSION) static_cast<void>(!!(EXPRESSION) || (SDK_DEBUG_BREAK(), 0))
#else
#define SDK_ASSERT(EXPRESSION) static_cast<void>(EXPRESSION)
#endif
#endif

#define _SDK_INTERNAL_CONCATENATE(LEFT, RIGHT) LEFT##RIGHT
#define _SDK_INTERNAL_UNPARENTHESIZE(...)      __VA_ARGS__
#define SDK_CONCATENATE(LEFT, RIGHT)           _SDK_INTERNAL_CONCATENATE(LEFT, RIGHT)
#define SDK_UNPARENTHESIZE(...)                _SDK_INTERNAL_UNPARENTHESIZE(__VA_ARGS__)

#define MEM_PAD(SIZE) \
\
private: \
	char SDK_CONCATENATE(pad_0, __COUNTER__)[SIZE]; \
\
public:

#define VIRTUAL_PAD() \
\
private: \
	virtual void SDK_CONCATENATE(unk, __COUNTER__)() = 0; \
\
public:

#pragma region COMMON_DEF

#ifndef GAME_NAME
#define GAME_NAME "csgo"
#endif

#define MAXPLAYERS 64

#ifdef _WIN32
#define ROOTBIN       "/bin/win64/"
#define GAMEBIN       "/" GAME_NAME "/bin/win64/"
#define MODULE_PREFIX ""
#define MODULE_EXT    ".dll"
#else
#define ROOTBIN       "/bin/linuxsteamrt64/"
#define GAMEBIN       "/" GAME_NAME "/bin/linuxsteamrt64/"
#define MODULE_PREFIX "lib"
#define MODULE_EXT    ".so"
#endif

#ifdef _WIN32
#define WIN_LINUX(win, linux) win
#else
#define WIN_LINUX(win, linux) linux
#endif

#define FILLMODULE(name)              MODULE_PREFIX name MODULE_EXT
#define FILLMODULE_FORCE_PREFIX(name) "lib" name MODULE_EXT
#define FILLMODULE_NO_PREFIX(name)    name MODULE_EXT

#if defined _WIN32
#define RETURN_ADDRESS _ReturnAddress()
#define FRAME_ADDRESS  _AddressOfReturnAddress()
#elif defined(__GNUC__)
#define RETURN_ADDRESS __builtin_return_address(0)
#define FRAME_ADDRESS  __builtin_frame_address(0) // @note: it isn't always what we're expecting, compiler dependent
#endif

void* Plat_LoadLibrary(const char* path, int mode = -1);
bool Plat_FreeLibrary(void* lib);
void* Plat_GetProcAddress(void* lib, const char* symbol);
void* Plat_GetModuleHandle(const char* path);
bool Plat_GetCommandArgument(const char* argName, char* buffer, size_t maxlength);

namespace LIB {
	constexpr inline const char* gamebin = GAMEBIN;
	constexpr inline const char* addons = GAME_NAME "/addons/cs2surf";

	// clang-format off
#define LIB_DECLARE_MODULE(sModule) \
	constexpr inline const char* sModule = FILLMODULE(#sModule)
	// clang-format on

	LIB_DECLARE_MODULE(engine2);
	LIB_DECLARE_MODULE(server);
	LIB_DECLARE_MODULE(tier0);
	LIB_DECLARE_MODULE(networksystem);
	LIB_DECLARE_MODULE(filesystem_stdio);
	LIB_DECLARE_MODULE(schemasystem);
	LIB_DECLARE_MODULE(matchmaking);
	LIB_DECLARE_MODULE(soundsystem);
	LIB_DECLARE_MODULE(resourcesystem);
	LIB_DECLARE_MODULE(steamnetworkingsockets);
	LIB_DECLARE_MODULE(worldrenderer);
} // namespace LIB

#pragma endregion
