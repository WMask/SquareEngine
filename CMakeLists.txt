############################################################################
##                             Square Engine                              ##
############################################################################

cmake_minimum_required(VERSION 3.21)

project(SquareEngine)

set(PROJECT_FILES_DIR "Projects/${CMAKE_SYSTEM_NAME}")
set(ENGINE_SOURCE_DIR "Code/Engine")
set(THIRD_PARTY_DIR "Code/ThirdParty")
set(SAMPLES_SOURCE_DIR "Code/Samples/${CMAKE_SYSTEM_NAME}")
set(CMAKE_SUPPRESS_REGENERATION true)

## Fetch third-party libs ##

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG        1.0.3
)

##	GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git

FetchContent_Declare(
	eigen
	GIT_REPOSITORY https://github.com/eigen-mirror/eigen.git
	GIT_TAG        5.0.1
)

message("--\n-- Downloading GL Math 1.0.3 ...\n--")

FetchContent_MakeAvailable(glm)
FetchContent_GetProperties(glm)

message("--\n-- Downloading Eigen Math 5.0.1 ...\n--")

FetchContent_MakeAvailable(eigen)
FetchContent_GetProperties(eigen)

## Add Engine libs ##

add_library(SCore SHARED
    ${ENGINE_SOURCE_DIR}/SConfig.h
    ${ENGINE_SOURCE_DIR}/Core/SCore.h
    ${ENGINE_SOURCE_DIR}/Core/SCoreModule.h
    ${ENGINE_SOURCE_DIR}/Core/SMacros.h
    ${ENGINE_SOURCE_DIR}/Core/SException.h
    ${ENGINE_SOURCE_DIR}/Core/STypes.h
    ${ENGINE_SOURCE_DIR}/Core/SMathTypes.h
    ${ENGINE_SOURCE_DIR}/Core/SMath.h
    ${ENGINE_SOURCE_DIR}/Core/SUtils.h
    ${ENGINE_SOURCE_DIR}/Core/SUtils.cpp
)
target_include_directories(SCore
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${glm_SOURCE_DIR}
		${eigen_SOURCE_DIR}
)
target_compile_features(SCore PUBLIC cxx_std_20)

add_library(SApplication SHARED
    ${ENGINE_SOURCE_DIR}/Application/SApplication.h
    ${ENGINE_SOURCE_DIR}/Application/SApplicationTypes.h
    ${ENGINE_SOURCE_DIR}/Application/SApplicationInterface.h
    ${ENGINE_SOURCE_DIR}/Application/SApplicationModule.h
    ${ENGINE_SOURCE_DIR}/Application/SApplicationModule.cpp
    ${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.cpp
    ${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.h
)
target_include_directories(SApplication
	PUBLIC
		${ENGINE_SOURCE_DIR}
)
target_compile_features(SApplication PUBLIC cxx_std_20)
target_link_libraries(SApplication SCore)
add_dependencies(SApplication SCore)

add_library(SRenderSystem SHARED
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystem.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemTypes.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemInterface.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemModule.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemModule.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/SWindowsUtils.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SWindowsUtils.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.h
)
target_include_directories(SRenderSystem
	PUBLIC
		${ENGINE_SOURCE_DIR}
)
target_compile_features(SRenderSystem PUBLIC cxx_std_20)
target_link_libraries(SRenderSystem SCore SApplication)
add_dependencies(SRenderSystem SCore SApplication)

## Add Sample projects ##

add_executable(HelloWorld
	${SAMPLES_SOURCE_DIR}/HelloWorld/HelloWorld.cpp
)
target_include_directories(HelloWorld
	PUBLIC
		${ENGINE_SOURCE_DIR}
)
target_compile_features(HelloWorld PUBLIC cxx_std_20)
target_link_libraries(HelloWorld SCore)
add_dependencies(HelloWorld SCore)

add_executable(HelloApplication
	${SAMPLES_SOURCE_DIR}/HelloApplication/HelloApplication.cpp
)
target_include_directories(HelloApplication
	PUBLIC
		${ENGINE_SOURCE_DIR}
)
target_compile_features(HelloApplication PUBLIC cxx_std_20)
target_link_libraries(HelloApplication SCore SApplication SRenderSystem)
add_dependencies(HelloApplication SCore SApplication SRenderSystem)

if (WIN32)
	target_compile_definitions(SCore PRIVATE
        UNICODE
        _UNICODE
	)
	target_compile_definitions(SApplication PRIVATE
        UNICODE
        _UNICODE
	)
	target_compile_definitions(SRenderSystem PRIVATE
        UNICODE
        _UNICODE
	)
	target_compile_definitions(HelloWorld PRIVATE
        UNICODE
        _UNICODE
	)
	target_compile_definitions(HelloApplication PRIVATE
        UNICODE
        _UNICODE
	)
	set_target_properties(HelloWorld PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
	)
	set_target_properties(HelloApplication PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	)
endif()
