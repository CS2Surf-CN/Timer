set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/hl2sdk-cs2 CACHE STRING "SDK root directory")
set(SDK_CONVAR ${SDK_ROOT}/tier1/convar.cpp CACHE STRING "SDK convar source")
set(SDK_MEMOVERRIDE ${SDK_ROOT}/public/tier0/memoverride.cpp CACHE STRING "SDK memory override source")
set(MM_ROOT ${CMAKE_CURRENT_LIST_DIR}/metamod-source CACHE STRING "Metamod root directory")
set(DUMP_ROOT ${CMAKE_CURRENT_LIST_DIR}/dumps CACHE STRING "Dumps directory")

if(WIN32)
	add_definitions(-DCOMPILER_MSVC -DCOMPILER_MSVC64 -D_WIN32 -D_WINDOWS -D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_SECURE_NO_DEPRECATE=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -D_HAS_EXCEPTIONS=0)

	set(CMAKE_CXX_FLAGS_RELWITHOD "${CMAKE_CXX_FLAGS_RELWITHOD} /Ob0 /Od /RTC1 /DNDEBUG /D_OD")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /wd4819 /wd4828 /wd5033 /permissive- /utf-8 /wd4005 /MP /W3 /TP /Oy-")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /DEBUG")
	set(CMAKE_SHARED_LINKER_FLAGS_RELWITHOD "${CMAKE_SHARED_LINKER_FLAGS_RELWITHOD} /DEBUG /NODEFAULTLIB:libcmtd")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:libcmt")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	add_link_options(
		"/SUBSYSTEM:WINDOWS"
		"kernel32.lib"
		"user32.lib"
		"gdi32.lib"
		"winspool.lib"
		"comdlg32.lib"
		"advapi32.lib"
		"shell32.lib"
		"ole32.lib"
		"oleaut32.lib"
		"uuid.lib"
		"odbc32.lib"
		"odbccp32.lib"
	)
else()
	add_definitions(-DCOMPILER_GCC -DGNUC -DPLATFORM_64BITS -DHAVE_STDINT_H -DLINUX -D_LINUX -DPOSIX -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=0)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dstrnicmp=strncasecmp -D_snprintf=snprintf -Dsprintf_s=snprintf")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp -Dstrncpy_s=strncpy")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMAX_PATH=PATH_MAX")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -fPIC")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -Wno-uninitialized -Wno-switch -Wno-unused -Wno-unused-result")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-virtual-dtor -Wno-overloaded-virtual -Wno-implicit-const-int-float-conversion")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null -Wno-write-strings -Wno-inconsistent-missing-override -Wno-ignored-attributes")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -Wno-reorder -Wno-implicit-exception-spec-mismatch -Wno-undefined-bool-conversion")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse -msse -fno-strict-aliasing -m64 -std=c++23")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics -fvisibility=hidden -fvisibility-inlines-hidden")

	add_link_options("-Wl,--no-undefined")
	add_link_options("-static-libstdc++")
	add_link_options("-lgcc_eh")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(NOT MSVC)
		add_compile_options(-g)
	endif()

	add_definitions(-D_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	if(NOT MSVC)
		add_compile_options(-O3)
	endif()

	add_definitions(-DNDEBUG)
endif()

add_definitions(-DMETA_IS_SOURCE2)

if(WIN32)
	file(GLOB SHARED_LIBS
		"${SDK_ROOT}/lib/public/win64/2015/*.lib"
		"${SDK_ROOT}/lib/public/win64/interfaces.lib"
		"${SDK_ROOT}/lib/public/win64/mathlib.lib"
		"${SDK_ROOT}/lib/public/win64/steam_api64.lib"
		"${SDK_ROOT}/lib/public/win64/tier1.lib"
		"${DUMP_ROOT}/lib/tier0/*.lib"
	)
else()
	file(GLOB SHARED_LIBS
		"${SDK_ROOT}/lib/linux64/*.a"
		"${SDK_ROOT}/lib/linux64/libsteam_api.so"
		"${SDK_ROOT}/lib/linux64/release/*.a"
		"${DUMP_ROOT}/lib/tier0/*.so"
	)
endif()

link_libraries(${SHARED_LIBS})

include_directories(
	${CMAKE_CURRENT_LIST_DIR}
	${SDK_ROOT}
	${SDK_ROOT}/thirdparty/protobuf-3.21.8/src
	${SDK_ROOT}/public
	${SDK_ROOT}/public/engine
	${SDK_ROOT}/public/mathlib
	${SDK_ROOT}/public/vstdlib
	${SDK_ROOT}/public/tier0
	${SDK_ROOT}/public/tier1
	${SDK_ROOT}/public/entity2
	${SDK_ROOT}/game/shared
	${SDK_ROOT}/game/server
	${SDK_ROOT}/common
	${MM_ROOT}/core
	${MM_ROOT}/core/sourcehook
	${MM_ROOT}/public
)
