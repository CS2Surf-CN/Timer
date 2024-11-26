cmake_minimum_required(VERSION 3.22)

project(cs2surf)

include("shared.cmake")

set(SURFTIMER_SRC "./src")

include_directories(
	${SURFTIMER_SRC}
	${SURFTIMER_SRC}/sdk/protobuf
	./vendor/libhv/include
	./vendor/libmem/include
	./vendor/libmodule
	./vendor/nlohmann
)

if(WIN32)
	file(GLOB SURFTIMER_LIBS 
			"./vendor/libmem/lib/Release/*.lib"
			"./vendor/libhv/lib/Release/*.lib"
	)
else()
	file(GLOB SURFTIMER_LIBS 
			"./vendor/libmem/lib/*.a"
			"./vendor/libhv/lib/*.a"
	)
endif()

set(LIBMODULE_DIR "./vendor/libmodule")
file(GLOB_RECURSE LIBMODULE_SOURCES "${LIBMODULE_DIR}/module.cpp" "${LIBMODULE_DIR}/*.h")
if(WIN32)
	list(APPEND LIBMODULE_SOURCES "${LIBMODULE_DIR}/module_windows.cpp")
else()
	list(APPEND LIBMODULE_SOURCES "${LIBMODULE_DIR}/module_linux.cpp")
endif()

file(GLOB_RECURSE ENTITY2_SOURCES "${SDK_ROOT}/entity2/entityidentity.cpp" "${SDK_ROOT}/entity2/entitysystem.cpp")
file(GLOB_RECURSE PROTOBUF_SOURCES "${SURFTIMER_SRC}/sdk/protobuf/*.pb.cc" "${SURFTIMER_SRC}/sdk/protobuf/*.pb.h")
file(GLOB_RECURSE SURFTIMER_SOURCES "${SURFTIMER_SRC}/*.cpp" "${SURFTIMER_SRC}/*.h")

add_definitions(-DHV_STATICLIB -DHAVE_MEMOVERRIDE_NEW_DELETE)

add_library(${PROJECT_NAME} SHARED ${SDK_CONVAR} ${LIBMODULE_SOURCES} ${ENTITY2_SOURCES} ${PROTOBUF_SOURCES} ${SURFTIMER_SOURCES})

if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE "ntdll.lib")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${SURFTIMER_LIBS})

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SURFTIMER_SRC}/pch.h")

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

set(OUT_DIR "NULL" CACHE PATH "SurfTimer binary directory for post-build file copy")

if (EXISTS ${OUT_DIR})
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:${PROJECT_NAME}>
			${OUT_DIR}
	)
endif()

