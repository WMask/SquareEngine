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

FetchContent_Declare(
	eigen
	GIT_REPOSITORY https://github.com/eigen-mirror/eigen.git
	GIT_TAG        5.0.1
)

FetchContent_Declare(
	entt
	GIT_REPOSITORY https://github.com/skypjack/entt.git
	GIT_TAG        v3.16.0
)

message("--\n-- Downloading GL Math 1.0.3 ...\n--")

FetchContent_MakeAvailable(glm)
FetchContent_GetProperties(glm)

message("--\n-- Downloading Eigen Math 5.0.1 ...\n--")

FetchContent_MakeAvailable(eigen)
FetchContent_GetProperties(eigen)

message("--\n-- Downloading Entt 3.16.0 ...\n--")

FetchContent_MakeAvailable(entt)
FetchContent_GetProperties(entt)

## Add Engine libs ##

add_library(SCore SHARED
    ${ENGINE_SOURCE_DIR}/SConfig.h
    ${ENGINE_SOURCE_DIR}/Core/CoreModule.h
    ${ENGINE_SOURCE_DIR}/Core/SCoreModule.h
    ${ENGINE_SOURCE_DIR}/Core/SMacros.h
    ${ENGINE_SOURCE_DIR}/Core/SCamera.h
    ${ENGINE_SOURCE_DIR}/Core/SDelegate.h
    ${ENGINE_SOURCE_DIR}/Core/SException.h
    ${ENGINE_SOURCE_DIR}/Core/SLogger.h
    ${ENGINE_SOURCE_DIR}/Core/STypes.h
    ${ENGINE_SOURCE_DIR}/Core/SMathTypes.h
    ${ENGINE_SOURCE_DIR}/Core/SMath.h
    ${ENGINE_SOURCE_DIR}/Core/SUtils.h
    ${ENGINE_SOURCE_DIR}/Core/SUtils.cpp
    ${ENGINE_SOURCE_DIR}/Core/ThreadPool/Fifo4.h
    ${ENGINE_SOURCE_DIR}/Core/ThreadPool/SThreadPoolInterface.h
    ${ENGINE_SOURCE_DIR}/Core/ThreadPool/SThreadPool.h
    ${ENGINE_SOURCE_DIR}/Core/ThreadPool/SThreadPool.cpp
)
target_include_directories(SCore
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${glm_SOURCE_DIR}
		${eigen_SOURCE_DIR}
)
target_compile_features(SCore PUBLIC cxx_std_20)


add_library(SWorld SHARED
    ${ENGINE_SOURCE_DIR}/World/WorldModule.h
    ${ENGINE_SOURCE_DIR}/World/SWorldModule.h
    ${ENGINE_SOURCE_DIR}/World/SWorldInterface.h
    ${ENGINE_SOURCE_DIR}/World/SWorld.h
    ${ENGINE_SOURCE_DIR}/World/SWorld.cpp
)
target_include_directories(SWorld
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${entt_SOURCE_DIR}/src
)
target_compile_features(SWorld PUBLIC cxx_std_20)
target_link_libraries(SWorld SCore)
add_dependencies(SWorld SCore)


add_library(SApplication SHARED
    ${ENGINE_SOURCE_DIR}/Application/ApplicationModule.h
    ${ENGINE_SOURCE_DIR}/Application/SApplicationModule.h
    ${ENGINE_SOURCE_DIR}/Application/SApplicationModule.cpp
    ${ENGINE_SOURCE_DIR}/Application/SApplicationTypes.h
    ${ENGINE_SOURCE_DIR}/Application/SApplicationInterface.h
    ${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.cpp
    ${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.h
)
target_include_directories(SApplication
	PUBLIC
		${ENGINE_SOURCE_DIR}
)
target_compile_features(SApplication PUBLIC cxx_std_20)
target_link_libraries(SApplication SCore SWorld)
add_dependencies(SApplication SCore SWorld)


add_library(SRenderSystem SHARED
    ${ENGINE_SOURCE_DIR}/RenderSystem/SECSComponents.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/RenderSystemModule.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemModule.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemModule.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemTypes.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/SRenderSystemInterface.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SWindowsUtils.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SWindowsUtils.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SDXShaderManager.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SDXShaderManager.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRendererDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRendererDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.cpp
)
target_include_directories(SRenderSystem
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${entt_SOURCE_DIR}/src
)
target_compile_features(SRenderSystem PUBLIC cxx_std_20)
target_link_libraries(SRenderSystem SCore SApplication SWorld)
add_dependencies(SRenderSystem SCore SApplication SWorld)

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


add_executable(SpritesSample
	${SAMPLES_SOURCE_DIR}/SpritesSample/SpritesSample.cpp
)
target_include_directories(SpritesSample
	PUBLIC
		${ENGINE_SOURCE_DIR}
)
target_compile_features(SpritesSample PUBLIC cxx_std_20)
target_link_libraries(SpritesSample SCore SApplication SRenderSystem SWorld)
add_dependencies(SpritesSample SCore SApplication SRenderSystem SWorld)


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
	target_compile_definitions(SpritesSample PRIVATE
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
	set_target_properties(SpritesSample PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	)
	source_group("Header Files\\ThreadPool" FILES
		${ENGINE_SOURCE_DIR}/Core/ThreadPool/Fifo4.h
		${ENGINE_SOURCE_DIR}/Core/ThreadPool/SThreadPool.h
		${ENGINE_SOURCE_DIR}/Core/ThreadPool/SThreadPoolInterface.h
	)
	source_group("Source Files\\ThreadPool" FILES
		${ENGINE_SOURCE_DIR}/Core/ThreadPool/SThreadPool.cpp
	)
	source_group("Header Files\\Windows" FILES
		${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.h
	)
	source_group("Source Files\\Windows" FILES
		${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.cpp
	)
	source_group("Header Files\\Windows" FILES
		${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SWindowsUtils.h
		${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SDXShaderManager.h
	)
	source_group("Source Files\\Windows" FILES
		${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SWindowsUtils.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/Windows/SDXShaderManager.cpp
	)
	source_group("Header Files\\DX11" FILES
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRendererDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.h
	)
	source_group("Source Files\\DX11" FILES
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRendererDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.cpp
	)
endif()
