cmake_minimum_required(VERSION 3.10)

project(Game)

set(CMAKE_CXX_STANDARD 23)

# SDL2 Libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_executable(Engine src/engine/main.cpp)

target_include_directories(Engine PUBLIC
    src/shared/
    src/renderer/
    src/engine/
  )

target_sources(Engine 
  PRIVATE 
    src/engine/data/Map.cpp
    src/engine/data/Sprite.cpp
    src/engine/data/Tile.cpp
    
    src/engine/entity/ScriptedDoodad.cpp
    
    src/engine/meta/UnitTable.cpp
    src/engine/meta/PortraitTable.cpp
    
    src/engine/script/IScriptEngine.cpp

    src/engine/video/codec/Vp9_Decoder.cpp
    src/engine/video/webm/ParseCallback.cpp
    src/engine/video/webm/WebmReader.cpp
    src/engine/video/Decoder.cpp
    src/engine/video/Video.cpp

    src/engine/view/UnitTransmission.cpp
    
    src/shared/audio/AudioManager.cpp
    src/shared/audio/Music.cpp
    
    src/shared/data/Common.cpp
    src/shared/data/Grp.cpp
    src/shared/data/Images.cpp
    src/shared/data/Palette.cpp
    src/shared/data/TextStrings.cpp
    src/shared/data/Tileset.cpp

    src/shared/filesystem/MpqArchive.cpp
    src/shared/filesystem/MpqFile.cpp
    src/shared/filesystem/StorageFile.cpp
    src/shared/filesystem/Storage.cpp

    src/shared/diagnostic/Clock.cpp
    src/shared/diagnostic/Image.cpp
  )

add_library(Renderer STATIC 

  src/renderer/vulkan/device/Device.cpp
  src/renderer/vulkan/device/Format.cpp
  src/renderer/vulkan/device/Framebuffer.cpp
  src/renderer/vulkan/device/Queue.cpp
  src/renderer/vulkan/device/SwapChain.cpp
  src/renderer/vulkan/memory/Buffer.cpp
  src/renderer/vulkan/memory/BufferAllocator.cpp
  src/renderer/vulkan/memory/Image.cpp
  src/renderer/vulkan/memory/MemoryManager.cpp
  src/renderer/vulkan/Api.cpp
  src/renderer/vulkan/Atlas.cpp
  src/renderer/vulkan/Command.cpp
  src/renderer/vulkan/DescriptorSetLayout.cpp
  src/renderer/vulkan/Config.cpp
  src/renderer/vulkan/Drawable.cpp
  src/renderer/vulkan/RenderPass.cpp
  src/renderer/vulkan/Sampler.cpp
  src/renderer/vulkan/Shader.cpp
  src/renderer/vulkan/SpritePacker.cpp
  src/renderer/vulkan/Vertex.cpp
  src/renderer/vulkan/VulkanGraphics.cpp
  src/renderer/vulkan/Window.cpp

  src/shared/data/Assets.cpp
  src/shared/data/Common.cpp
  src/shared/data/Grp.cpp
  src/shared/data/Images.cpp
  src/shared/data/Palette.cpp
  src/shared/data/TextStrings.cpp
  src/shared/data/Tileset.cpp

  src/shared/diagnostic/Clock.cpp
  src/shared/diagnostic/Image.cpp

  src/shared/filesystem/MpqArchive.cpp
  src/shared/filesystem/MpqFile.cpp
  src/shared/filesystem/StorageFile.cpp
  src/shared/filesystem/Storage.cpp
  )

target_include_directories(Renderer 
  PUBLIC
    src/shared/
    src/renderer/
  )

# Find Winsock library
find_path(WINSOCK_INCLUDE_DIR WinSock2.h)
if(MSVC)
  find_library(WINSOCK_LIBRARY mswsock.lib)
  find_library(WINSOCK2_LIBRARY ws2_32.lib)
  find_library(WINSOCK2_LIBRARY bcrypt.lib)
else()
  find_library(WINSOCK_LIBRARY mswsock)
  find_library(WINSOCK2_LIBRARY ws2_32)
  find_library(WINSOCK2_LIBRARY bcrypt)
endif()

# Handle the REQUIRED argument and set WINSOCK_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WinSock DEFAULT_MSG WINSOCK_LIBRARY WINSOCK2_LIBRARY WINSOCK_INCLUDE_DIR)

mark_as_advanced(WINSOCK_INCLUDE_DIR)
mark_as_advanced(WINSOCK_LIBRARY)
mark_as_advanced(WINSOCK2_LIBRARY)

if(WINSOCK_FOUND)
  add_definitions(-DWINSOCK_SUPPORT)
  set(WINSOCK_LIBRARIES ${WINSOCK_LIBRARY} ${WINSOCK2_LIBRARY})
endif()

if(MINGW)
  set(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} -lwsock32 -lws2_32 -lbcrypt")
  set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lwsock32 -lws2_32 -lbcrypt")
endif()

# Casc Library
set(CASC_BUILD_STATIC_LIB ON)
set(CASC_BUILD_SHARED_LIB OFF)

add_subdirectory(CascLib/)

# StormLib
add_subdirectory(StormLib/)

# Boost
find_package(Boost REQUIRED)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(Engine ${Boost_LIBRARIES})
endif()

target_link_libraries(Engine 
  PRIVATE Renderer
  PRIVATE casc_static
  PRIVATE storm
  PRIVATE SDL2::SDL2-static
  PRIVATE SDL2_mixer
  PRIVATE libpng
  PRIVATE libvpx
  PRIVATE libwebm)


target_link_libraries(Renderer 
  PRIVATE casc_static
  PRIVATE storm
  PRIVATE SDL2::SDL2-static
  PRIVATE vulkan-1)

file(COPY static/pal_frag.spv static/pal_vert.spv static/tex_frag.spv static/tex_vert.spv
     DESTINATION shaders/)