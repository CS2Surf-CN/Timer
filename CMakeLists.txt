cmake_minimum_required(VERSION 3.28)

project(cs2surf)

include("shared.cmake")

set(SURFTIMER_SRC "./src")

include_directories(
	${SURFTIMER_SRC}
	${SURFTIMER_SRC}/sdk/protobuf
	./vendor/libhv/include
	./vendor/libmem/include
	./vendor/libmodule/include
	./vendor/nlohmann
	./vendor/fmt/include
	./vendor/polyhook/include
	./vendor/zlib/include
)

file(GLOB SURFTIMER_LIBS 
    "./vendor/libmem/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "./vendor/libmodule/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "./vendor/libhv/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "./vendor/polyhook/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "./vendor/zlib/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

file(GLOB_RECURSE SDK_SOURCES 
		"${SDK_CONVAR}"
		"${SDK_ROOT}/entity2/entityidentity.cpp" 
		"${SDK_ROOT}/entity2/entitysystem.cpp" 
		"${SDK_ROOT}/entity2/entitykeyvalues.cpp"
		"${SDK_ROOT}/tier1/keyvalues3.cpp"
)
file(GLOB_RECURSE PROTOBUF_SOURCES "${SURFTIMER_SRC}/sdk/protobuf/*.pb.cc" "${SURFTIMER_SRC}/sdk/protobuf/*.pb.h")
file(GLOB_RECURSE SURFTIMER_SOURCES "${SURFTIMER_SRC}/core/adminmanager.cpp"
									"${SURFTIMER_SRC}/core/gamedata.cpp"
									"${SURFTIMER_SRC}/core/concmdmanager.cpp"
									"${SURFTIMER_SRC}/core/core.cpp"
									"${SURFTIMER_SRC}/core/core.h"
									"${SURFTIMER_SRC}/core/memory.cpp"
									"${SURFTIMER_SRC}/core/interfaces.cpp"
									"${SURFTIMER_SRC}/core/forwards.cpp"
									"${SURFTIMER_SRC}/sdk/schema.cpp"
									"${SURFTIMER_SRC}/sdk/entity/cbaseentity.cpp"
									"${SURFTIMER_SRC}/sdk/entity/cgameresourceservice.cpp"
)
file(GLOB_RECURSE SURFTIMER_MODULES "${SURFTIMER_SRC}/core/adminmanager.cppm" 
									"${SURFTIMER_SRC}/core/gamedata.cppm" 
									"${SURFTIMER_SRC}/core/concmdmanager.cppm"
									"${SURFTIMER_SRC}/core/memory.cppm"
									"${SURFTIMER_SRC}/core/interfaces.cppm"
									"${SURFTIMER_SRC}/core/forwards.cppm"
									"${SURFTIMER_SRC}/core/core.cppm"
)

add_definitions(-DHV_STATICLIB -DHAVE_MEMOVERRIDE_NEW_DELETE -DFMT_HEADER_ONLY -DNOMINMAX)

add_library(${PROJECT_NAME} SHARED ${SDK_SOURCES} ${PROTOBUF_SOURCES})

target_sources(${PROJECT_NAME} PRIVATE ${SURFTIMER_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES FILES ${SURFTIMER_MODULES})

if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE "ntdll.lib")
else()
	find_package(OpenSSL REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${SURFTIMER_LIBS})

#target_precompile_headers(${PROJECT_NAME} PRIVATE "${SURFTIMER_SRC}/pch.h")

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

set(OUT_DIR "" CACHE PATH "SurfTimer binary directory for post-build file copy")

if (NOT ${OUT_DIR} STREQUAL "")
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:${PROJECT_NAME}>
			${OUT_DIR}
	)
endif()

