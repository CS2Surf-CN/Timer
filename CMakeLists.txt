cmake_minimum_required(VERSION 3.22)

project(cs2surf)

include("shared.cmake")

set(SURFTIMER_SRC "${CMAKE_CURRENT_LIST_DIR}/src")
set(SURFTIMER_VENDOR "${CMAKE_CURRENT_LIST_DIR}/vendor")

include_directories(
	${SURFTIMER_SRC}
	${SURFTIMER_SRC}/sdk/protobuf
	${SURFTIMER_VENDOR}/libhv/include
	${SURFTIMER_VENDOR}/libmodule/include
	${SURFTIMER_VENDOR}/nlohmann
	${SURFTIMER_VENDOR}/fmt/include
	${SURFTIMER_VENDOR}/safetyhook/include
	${SURFTIMER_VENDOR}/zlib/include
)

file(GLOB SURFTIMER_LIBS 
	"${SURFTIMER_VENDOR}/libmodule/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"${SURFTIMER_VENDOR}/libhv/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"${SURFTIMER_VENDOR}/zlib/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

set(SURFTIMER_ORDERED_LIBS 
	"${SURFTIMER_VENDOR}/safetyhook/lib/safetyhook${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"${SURFTIMER_VENDOR}/safetyhook/lib/Zydis${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"${SURFTIMER_VENDOR}/safetyhook/lib/Zycore${CMAKE_STATIC_LIBRARY_SUFFIX}"
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

add_definitions(-DHV_STATICLIB -DZYDIS_STATIC_BUILD -DHAVE_MEMOVERRIDE_NEW_DELETE -DFMT_HEADER_ONLY -DNOMINMAX)

add_library(${PROJECT_NAME} SHARED ${SDK_SOURCES} ${PROTOBUF_SOURCES} ${SURFTIMER_SOURCES})

if(WIN32)
else()
	find_package(OpenSSL REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${SURFTIMER_LIBS} ${SURFTIMER_ORDERED_LIBS})

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

