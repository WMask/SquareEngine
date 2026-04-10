############################################################################
##                             Square Engine                              ##
############################################################################

cmake_minimum_required(VERSION 3.21)

project(SquareEngine)

## Add configuration ##

set(PROJECT_FILES_DIR "Projects/${CMAKE_SYSTEM_NAME}")
set(ENGINE_SOURCE_DIR "Code/Engine")
set(SHADERS_DIR "Code/Shaders")
set(SAMPLES_SOURCE_DIR "Code/Samples/${CMAKE_SYSTEM_NAME}")
set(TOOLS_SOURCE_DIR "Code/Tools")
set(CMAKE_SUPPRESS_REGENERATION true)

set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_TOOLS OFF CACHE BOOL "" FORCE)

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

FetchContent_Declare(
	json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG        v3.12.0
)

FetchContent_Declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb.git
	GIT_TAG        origin/master
)

FetchContent_Declare(
	ufbx
	GIT_REPOSITORY https://github.com/ufbx/ufbx.git
	GIT_TAG        v0.21.3
)

FetchContent_Declare(
	DirectXTK
	GIT_REPOSITORY https://github.com/microsoft/DirectXTK.git
	GIT_TAG        mar2026
)

FetchContent_Declare(
	zlib
	GIT_REPOSITORY https://github.com/madler/zlib.git
	GIT_TAG        v1.3.2
)

FetchContent_Declare(
	png
	GIT_REPOSITORY https://github.com/pnggroup/libpng.git
	GIT_TAG        v1.6.56
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

message("--\n-- Downloading nlohmann/json 3.12.0 ...\n--")

FetchContent_MakeAvailable(json)
FetchContent_GetProperties(json)

message("--\n-- Downloading stb/truetype ...\n--")

FetchContent_MakeAvailable(stb)
FetchContent_GetProperties(stb)

message("--\n-- Downloading ufbx 0.21.3 ...\n--")

FetchContent_MakeAvailable(ufbx)
FetchContent_GetProperties(ufbx)

message("--\n-- Downloading DirectXTK mar2026 ...\n--")

FetchContent_MakeAvailable(DirectXTK)
FetchContent_GetProperties(DirectXTK)

message("--\n-- Downloading zlib 1.3.2 ...\n--")

FetchContent_MakeAvailable(zlib)
FetchContent_GetProperties(zlib)

set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR})
set(ZLIB_LIBRARY ${zlib_BINARY_DIR})

message("--\n-- Downloading libpng 1.6.56 ...\n--")

FetchContent_MakeAvailable(png)
FetchContent_GetProperties(png)

set(PNG_PNG_INCLUDE_DIR ${png_SOURCE_DIR})
set(PNG_LIBRARY ${png_BINARY_DIR})

find_package(PNG REQUIRED)


## Add Engine libs ##

add_library(ufbx STATIC
    ${ufbx_SOURCE_DIR}/ufbx.h
    ${ufbx_SOURCE_DIR}/ufbx.c
)
target_include_directories(ufbx
	PUBLIC
		${ufbx_SOURCE_DIR}
)
target_compile_features(ufbx PUBLIC cxx_std_20)


add_library(SCore SHARED
    ${ENGINE_SOURCE_DIR}/SConfig.h
    ${ENGINE_SOURCE_DIR}/Core/CoreModule.h
    ${ENGINE_SOURCE_DIR}/Core/SCoreModule.h
    ${ENGINE_SOURCE_DIR}/Core/SMacros.h
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
		${entt_SOURCE_DIR}/src
		${json_SOURCE_DIR}/include
		${ufbx_SOURCE_DIR}
		${zlib_SOURCE_DIR}
		${png_SOURCE_DIR}
		${png_BINARY_DIR}
)
target_compile_features(SCore PUBLIC cxx_std_20)
target_link_directories(SCore PRIVATE
    ${zlib_BINARY_DIR}
    ${png_BINARY_DIR}
)
target_link_libraries(SCore PRIVATE
    $<$<CONFIG:Debug>:ufbx zsd libpng16_staticd>
    $<$<CONFIG:Release>:ufbx zs libpng16_static>
)
add_dependencies(SCore ufbx zlib png_static)


add_library(SWorld SHARED
    ${ENGINE_SOURCE_DIR}/World/WorldModule.h
    ${ENGINE_SOURCE_DIR}/World/SWorldModule.h
    ${ENGINE_SOURCE_DIR}/World/SWorldInterface.h
    ${ENGINE_SOURCE_DIR}/World/SWorld.h
    ${ENGINE_SOURCE_DIR}/World/SWorld.cpp
    ${ENGINE_SOURCE_DIR}/World/SFontInterface.h
    ${ENGINE_SOURCE_DIR}/World/SFontSystem.h
    ${ENGINE_SOURCE_DIR}/World/SFontSystem.cpp
    ${ENGINE_SOURCE_DIR}/World/SGuiInterface.h
    ${ENGINE_SOURCE_DIR}/World/SGuiSystem.h
    ${ENGINE_SOURCE_DIR}/World/SGuiSystem.cpp
    ${ENGINE_SOURCE_DIR}/World/SCamera.h
)
target_include_directories(SWorld
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${entt_SOURCE_DIR}/src
		${json_SOURCE_DIR}/include
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
    ${ENGINE_SOURCE_DIR}/Application/SLocalizationInterface.h
    ${ENGINE_SOURCE_DIR}/Application/SLocalization.h
    ${ENGINE_SOURCE_DIR}/Application/SLocalization.cpp
    ${ENGINE_SOURCE_DIR}/Application/SInputInterface.h
    ${ENGINE_SOURCE_DIR}/Application/SInputSystem.h
    ${ENGINE_SOURCE_DIR}/Application/SInputSystem.cpp
    ${ENGINE_SOURCE_DIR}/Application/SAppConfig.h
    ${ENGINE_SOURCE_DIR}/Application/SAppConfig.cpp
    ${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.cpp
    ${ENGINE_SOURCE_DIR}/Application/Windows/SWindowsApplication.h
)
target_include_directories(SApplication
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${json_SOURCE_DIR}/include
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
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshRenderSystemDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshRenderSystemDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextRenderSystemDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextRenderSystemDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRenderSystemDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRenderSystemDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STexturedSpriteRenderSystemDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STexturedSpriteRenderSystemDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SFrameAnimSpriteRenderSystemDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SFrameAnimSpriteRenderSystemDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextureManagerDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextureManagerDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshManagerDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshManagerDX11.cpp
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.h
    ${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.cpp
)
target_include_directories(SRenderSystem
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${entt_SOURCE_DIR}/src
		${DirectXTK_SOURCE_DIR}/Inc
)
target_compile_features(SRenderSystem PUBLIC cxx_std_20)
target_link_libraries(SRenderSystem SCore SApplication SWorld DirectXTK)
add_dependencies(SRenderSystem SCore SApplication SWorld DirectXTK)

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


add_executable(StaticMeshSample
	${SAMPLES_SOURCE_DIR}/StaticMeshSample/StaticMeshSample.cpp
)
target_include_directories(StaticMeshSample
	PUBLIC
		${ENGINE_SOURCE_DIR}
)
target_compile_features(StaticMeshSample PUBLIC cxx_std_20)
target_link_libraries(StaticMeshSample SCore SApplication SRenderSystem)
add_dependencies(StaticMeshSample SCore SApplication SRenderSystem)


add_executable(FontTool
	${TOOLS_SOURCE_DIR}/FontTool/FontTool.cpp
)
target_include_directories(FontTool
	PUBLIC
		${ENGINE_SOURCE_DIR}
		${stb_SOURCE_DIR}
)
target_compile_features(FontTool PUBLIC cxx_std_20)
target_link_libraries(FontTool SCore)


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
	target_compile_definitions(StaticMeshSample PRIVATE
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
	set_target_properties(StaticMeshSample PROPERTIES
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
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SFrameAnimSpriteRenderSystemDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STexturedSpriteRenderSystemDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRenderSystemDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextRenderSystemDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshRenderSystemDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextureManagerDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshManagerDX11.h
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.h
	)
	source_group("Source Files\\DX11" FILES
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SFrameAnimSpriteRenderSystemDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STexturedSpriteRenderSystemDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SColoredSpriteRenderSystemDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextRenderSystemDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshRenderSystemDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SConstantBuffersDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/STextureManagerDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SMeshManagerDX11.cpp
		${ENGINE_SOURCE_DIR}/RenderSystem/DX11/SRenderSystemDX11.cpp
	)
endif()
