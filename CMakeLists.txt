cmake_minimum_required(VERSION 3.22)

project(cs2surf)

include("shared.cmake")

set(SURFTIMER_SRC "./src")

include_directories(
	${SURFTIMER_SRC}
	${SURFTIMER_SRC}/sdk/protobuf
	./vendor/libhv/include
	./vendor/libmodule/include
	./vendor/nlohmann
	./vendor/fmt/include
	./vendor/polyhook/include
	./vendor/safetyhook/include
	./vendor/zlib/include
)

file(GLOB SURFTIMER_LIBS 
	"./vendor/libmodule/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"./vendor/libhv/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"./vendor/polyhook/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"./vendor/safetyhook/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
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
file(GLOB_RECURSE SURFTIMER_SOURCES "${SURFTIMER_SRC}/*.cpp" "${SURFTIMER_SRC}/*.h")

add_definitions(-DHV_STATICLIB -DHAVE_MEMOVERRIDE_NEW_DELETE -DFMT_HEADER_ONLY -DNOMINMAX)

add_library(${PROJECT_NAME} SHARED ${SDK_SOURCES} ${PROTOBUF_SOURCES} ${SURFTIMER_SOURCES})

if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE "ntdll.lib")
else()
	find_package(OpenSSL REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${SURFTIMER_LIBS})

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SURFTIMER_SRC}/pch.h")

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

