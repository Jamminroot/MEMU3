cmake_minimum_required(VERSION 3.10)

project(MEMU3 C CXX)

set(TARGET_NAME MEMU3)

set(CMAKE_CXX_STANDARD 20)
set(SUGGESTED_STACK_SIZE 10000000)

string(TOUPPER "${CMAKE_BUILD_TYPE}" VARIANT)

add_definitions(-DUNICODE)

message("Build variant:\t ${VARIANT}")

if (VARIANT STREQUAL "DEBUG")
    add_definitions(-DDEBUG=1)
else ()

endif ()

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")

#string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

set(SOURCE_FILES
        sources/Main.cpp
        sources/Overlay.cpp
        sources/Rect.cpp
        sources/ScreenshotData.cpp
        sources/ScreenshotFactory.cpp
        sources/Manager.cpp
        sources/AimAssistant.cpp
        sources/Coords.cpp
        sources/InputController.cpp
        sources/Utils.cpp
        sources/TtlStringCollection.cpp
        )

configure_file(Icons/Aim.bmp Aim.bmp COPYONLY)
configure_file(Icons/Trigger.bmp Trigger.bmp COPYONLY)
configure_file(Icons/Hanzo.bmp Hanzo.bmp COPYONLY)
configure_file(Icons/Flick.bmp Flick.bmp COPYONLY)

configure_file(SampleConfigs/red.colorset red.colorset COPYONLY)
configure_file(SampleConfigs/red+neon.colorset red+neon.colorset COPYONLY)
configure_file(SampleConfigs/neon.colorset neon.colorset COPYONLY)

add_executable(${TARGET_NAME} ${SOURCE_FILES})

string(REPLACE "/W4" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /WX")

if (MSVC)
    message("MSVC Compiler detected. Setting stack size to ${SUGGESTED_STACK_SIZE}b")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${SUGGESTED_STACK_SIZE}")
else ()
    message("Other linker detected. Make sure to increase stack size is increased to ${SUGGESTED_STACK_SIZE}b!")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,${SUGGESTED_STACK_SIZE}")
endif ()

include_directories(Interception/)

if (TARGET_64)
    add_definitions(-DTARGET_64=1)
    link_directories(Interception/x64/)
else ()
    link_directories(Interception/x86/)
endif ()

# Direct X

set(DirectX_ROOT_DIR "$ENV{ProgramFiles\(x86\)}/Microsoft DirectX SDK (June 2010)")
set(DirectX_INCLUDE_DIRS "${DirectX_ROOT_DIR}/Include")

#get_filename_component(DirectX_ROOT_DIR "${DirectX_INCLUDE_DIRS}/.." ABSOLUTE)

if (TARGET_64)
    set(DirectX_LIBRARY_PATHS "${DirectX_ROOT_DIR}/Lib/x64")
else ()
    set(DirectX_LIBRARY_PATHS "${DirectX_ROOT_DIR}/Lib/x86" "${DirectX_ROOT_DIR}/Lib")
endif ()

message("DirectX path:\n${DirectX_ROOT_DIR}")

find_library(DirectX_D3D9_LIBRARY d3d9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(DirectX_D3DX9_LIBRARY d3dx9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
set(DirectX_LIBRARIES ${DirectX_D3D9_LIBRARY} ${DirectX_D3DX9_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set DirectX_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DirectX DEFAULT_MSG DirectX_ROOT_DIR DirectX_LIBRARIES DirectX_INCLUDE_DIRS)
mark_as_advanced(DirectX_INCLUDE_DIRS DirectX_D3D9_LIBRARY DirectX_D3DX9_LIBRARY)
include_directories(${DirectX_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} "${DirectX_LIBRARY_PATHS}/d3d9.lib")
target_link_libraries(${TARGET_NAME} "${DirectX_LIBRARY_PATHS}/d3dx9.lib")

# Output flags

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message("CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")