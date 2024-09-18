cmake_minimum_required(VERSION 3.10)

project(Game)

set(CMAKE_CXX_STANDARD 23)

# SDL2 Libraries
find_package(SDL2 REQUIRED)

add_executable(Engine src/engine/main.cpp)

target_sources(Engine 
  PRIVATE 
    src/engine/data/Common.cpp
    src/engine/data/Grp.cpp
    src/engine/data/Images.cpp
    src/engine/data/Map.cpp
    src/engine/data/Palette.cpp
    src/engine/data/Sprite.cpp
    src/engine/data/TextStrings.cpp
    src/engine/data/Tile.cpp

    src/engine/entity/ScriptedDoodad.cpp

    src/engine/filesystem/MpqArchive.cpp
    src/engine/filesystem/MpqFile.cpp
    src/engine/filesystem/StorageFile.cpp
    src/engine/filesystem/Storage.cpp

    src/engine/meta/Unit.cpp

    src/engine/script/IScriptEngine.cpp
  )

add_library(Render 
  PRIVATE
    src/renderer/render/Atlas.cpp
    
  SHARED
    src/renderer/render/Atlas.hpp
    src/renderer/render/Palette.hpp
    src/renderer/render/Tileset.hpp
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
  PRIVATE casc_static
  PRIVATE storm
  PRIVATE SDL2::SDL2-static)