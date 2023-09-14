cmake_minimum_required(VERSION 3.10)

project(MEMU3 C CXX)

set(TARGET_NAME MEMU3)

set(CMAKE_CXX_STANDARD 20)
set(SUGGESTED_STACK_SIZE 10000000)

if (TARGET_64)
    set(ARCH_STRING x64)
    add_definitions(-DTARGET_64=1)
    message("64 bit target detected")
else ()
    set(ARCH_STRING x86)
    message("32 bit target detected")
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" VARIANT)

add_definitions(-DUNICODE)
add_definitions(-DCMAKELISTS)

message("Build variant:\t ${VARIANT}")

if (VARIANT STREQUAL "DEBUG")
    add_definitions(-DDEBUG=1)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")

#string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

set(SOURCE_FILES
        MEMU3.rc
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

# Copy maps folder on every build
file(COPY "maps" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

configure_file(Configs/red.colorset red.colorset COPYONLY)
configure_file(Configs/red+neon.colorset red+neon.colorset COPYONLY)
configure_file(Configs/neon.colorset neon.colorset COPYONLY)
configure_file(Configs/MEMU3.config MEMU3.config COPYONLY)

add_executable(${TARGET_NAME} ${SOURCE_FILES})

#set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /WX")

#string(REPLACE "/W4" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (MSVC)
    message("MSVC Compiler detected. Setting stack size to ${SUGGESTED_STACK_SIZE}b")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${SUGGESTED_STACK_SIZE}")

    set(MSVC_ROOT "$ENV{ProgramFiles\(x86\)}\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.29.30037")
    set(MSVC_LIB_ROOT "${MSVC_ROOT}\\lib\\${ARCH_STRING}")
    set(MSVC_INCLUDE_ROOT "${MSVC_ROOT}\\include")

    target_link_directories(${TARGET_NAME} PUBLIC ${MSVC_LIB_ROOT})
    include_directories(${MSVC_INCLUDE_ROOT})
else ()
    message("Other linker detected. Make sure to increase stack size is increased to ${SUGGESTED_STACK_SIZE}b!")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,${SUGGESTED_STACK_SIZE}")
endif ()

include_directories(Interception/)

link_directories(Interception\\${ARCH_STRING})

file(COPY "Interception\\${ARCH_STRING}\\interception.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

# Direct X

set(DirectX_INCLUDE_DIRS "$ENV{DXSDK_DIR}Include")
set(DirectX_LIB_ROOT_DIR "$ENV{DXSDK_DIR}Lib\\${ARCH_STRING}")
message("DX ENV Variable : $ENV{DXSDK_DIR}")
message("DirectX_LIB_ROOT_DIR : ${DirectX_LIB_ROOT_DIR}")

include_directories(${DirectX_INCLUDE_DIRS})

#add_definitions(-DD3D9LIBPATH=${DirectX_LIB_ROOT_DIR}\\${ARCH_STRING}\\d3d9.lib)
#add_definitions(-DD3DX9LIBPATH=${DirectX_LIB_ROOT_DIR}\\${ARCH_STRING}\\d3dx9.lib)

#link_directories(${DirectX_LIB_ROOT_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC \"${DirectX_LIB_ROOT_DIR}\\d3d9\")
target_link_libraries(${TARGET_NAME} PUBLIC \"${DirectX_LIB_ROOT_DIR}\\d3dx9\")

#target_link_libraries(${TARGET_NAME} \"${DirectX_LIB_ROOT_DIR}/d3d9.lib\" \"${DirectX_LIB_ROOT_DIR}/d3dx9.lib\")

# Output flags
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message("CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")