cmake_minimum_required(VERSION 3.10)

project(MEMU3 C CXX)

set(CMAKE_CXX_STANDARD 17)
set(SUGGESTED_STACK_SIZE 10000000)
set(EXECUTABLE_NAME MEMU3)

string(TOUPPER "${CMAKE_BUILD_TYPE}" VARIANT)

add_definitions(-DUNICODE)

message("Build variant:\t ${VARIANT}")

if (VARIANT STREQUAL "DEBUG")
    add_definitions(-DDEBUG=1)
else ()

endif ()


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("MSVC Compiler detected. Setting stack size to ${SUGGESTED_STACK_SIZE}b")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${SUGGESTED_STACK_SIZE}")
else ()
    message("Other linker detected. Make sure to increase stack size is increased to ${SUGGESTED_STACK_SIZE}b!")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,${SUGGESTED_STACK_SIZE}")
endif ()

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")

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
        sources/TtlStringCollection.cpp headers/Defines.h)


add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})


#[[

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /AI\"../Overlay_Library\" " )
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} /AI\"../Overlay_Library\" " )

string(REPLACE "/RTC1" " " CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")


#set_target_properties(${PROJECT_NAME} PROPERTIES COMMON_LANGUAGE_RUNTIME "")
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/clr")
# Interception
]]



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

message("DirectX paths:\n${DirectX_ROOT_DIR}\n${DirectX_INCLUDE_DIRS}\n${DirectX_LIBRARY_PATHS}")

find_library(DirectX_D3D9_LIBRARY d3d9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(DirectX_D3DX9_LIBRARY d3dx9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
set(DirectX_LIBRARIES ${DirectX_D3D9_LIBRARY} ${DirectX_D3DX9_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set DirectX_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DirectX DEFAULT_MSG DirectX_ROOT_DIR DirectX_LIBRARIES DirectX_INCLUDE_DIRS)
mark_as_advanced(DirectX_INCLUDE_DIRS DirectX_D3D9_LIBRARY DirectX_D3DX9_LIBRARY)
include_directories(${DirectX_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE_NAME} "${DirectX_LIBRARY_PATHS}/d3d9.lib")
target_link_libraries(${EXECUTABLE_NAME} "${DirectX_LIBRARY_PATHS}/d3dx9.lib")


#[[
if (WIN32)

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (DirectX_ARCHITECTURE x64)
    else ()
        set (DirectX_ARCHITECTURE x86)
    endif ()

    # Can't use "$ENV{ProgramFiles(x86)}" to avoid violating CMP0053.  See
    # http://public.kitware.com/pipermail/cmake-developers/2014-October/023190.html
    set (ProgramFiles_x86 "ProgramFiles(x86)")
    if ("$ENV{${ProgramFiles_x86}}")
        set (ProgramFiles "$ENV{${ProgramFiles_x86}}")
    else ()
        set (ProgramFiles "$ENV{ProgramFiles}")
    endif ()

    find_path (DirectX_ROOT_DIR
            Include/d3dx9.h
            PATHS
            "$ENV{DXSDK_DIR}"
            "${ProgramFiles}/Microsoft DirectX SDK (June 2010)"
            "${ProgramFiles}/Microsoft DirectX SDK (February 2010)"
            "${ProgramFiles}/Microsoft DirectX SDK (March 2009)"
            "${ProgramFiles}/Microsoft DirectX SDK (August 2008)"
            "${ProgramFiles}/Microsoft DirectX SDK (June 2008)"
            "${ProgramFiles}/Microsoft DirectX SDK (March 2008)"
            "${ProgramFiles}/Microsoft DirectX SDK (November 2007)"
            "${ProgramFiles}/Microsoft DirectX SDK (August 2007)"
            "${ProgramFiles}/Microsoft DirectX SDK"
            DOC "DirectX SDK root directory"
            )
    if (DirectX_ROOT_DIR)
        set (DirectX_INC_SEARCH_PATH "${DirectX_ROOT_DIR}/Include")
        set (DirectX_LIB_SEARCH_PATH "${DirectX_ROOT_DIR}/Lib/${DirectX_ARCHITECTURE}")
        set (DirectX_BIN_SEARCH_PATH "${DirectX_ROOT_DIR}/Utilities/bin/x86")
    endif ()

    # With VS 2011 and Windows 8 SDK, the DirectX SDK is included as part of
    # the Windows SDK.
    #
    # See also:
    # - http://msdn.microsoft.com/en-us/library/windows/desktop/ee663275.aspx
    if (DEFINED MSVC_VERSION AND NOT ${MSVC_VERSION} LESS 1700)
        set (USE_WINSDK_HEADERS TRUE)
    endif ()

    # Find a header in the DirectX SDK
    macro (find_dxsdk_header var_name header)
        set (include_dir_var "DirectX_${var_name}_INCLUDE_DIR")
        set (include_found_var "DirectX_${var_name}_INCLUDE_FOUND")
        find_path (${include_dir_var} ${header}
                HINTS ${DirectX_INC_SEARCH_PATH}
                DOC "The directory where ${header} resides"
                CMAKE_FIND_ROOT_PATH_BOTH
                )
        if (${include_dir_var})
            set (${include_found_var} TRUE)
            message (STATUS ${var_name}_INC "Found ${header} header: ${${include_dir_var}}/${header}" "[${${include_dir_var}}]")
        endif ()
        mark_as_advanced (${include_found_var})
    endmacro ()

    # Find a library in the DirectX SDK
    macro (find_dxsdk_library var_name library)
        # DirectX SDK
        set (library_var "DirectX_${var_name}_LIBRARY")
        find_library (${library_var} ${library}
                HINTS ${DirectX_LIB_SEARCH_PATH}
                DOC "The directory where ${library} resides"
                CMAKE_FIND_ROOT_PATH_BOTH
                )
        if (${library_var})
            message (STATUS ${var_name}_LIB "Found ${library} library: ${${library_var}}" "[${${library_var}}]")
        endif ()
        mark_as_advanced (${library_var})
    endmacro ()

    # Find a header in the Windows SDK
    macro (find_winsdk_header var_name header)
        if (USE_WINSDK_HEADERS)
            # Windows SDK
            set (include_dir_var "DirectX_${var_name}_INCLUDE_DIR")
            set (include_found_var "DirectX_${var_name}_INCLUDE_FOUND")
            #check_include_file_cxx (${header} ${include_found_var})
            set (${include_dir_var})
            mark_as_advanced (${include_found_var})
        else ()
            find_dxsdk_header (${var_name} ${header})
        endif ()
    endmacro ()

    # Find a library in the Windows SDK
    macro (find_winsdk_library var_name library)
        if (USE_WINSDK_HEADERS)
            # XXX: We currently just assume the library exists
            set (library_var "DirectX_${var_name}_LIBRARY")
            set (${library_var} ${library})
            mark_as_advanced (${library_var})
        else ()
            find_dxsdk_library (${var_name} ${library})
        endif ()
    endmacro ()

    # Combine header and library variables into an API found variable
    macro (find_combined var_name inc_var_name lib_var_name)
        if (DirectX_${inc_var_name}_INCLUDE_FOUND AND DirectX_${lib_var_name}_LIBRARY)
            set (DirectX_${var_name}_FOUND 1)
            message (STATUS ${var_name} "Found ${var_name} API" "[${DirectX_${lib_var_name}_LIBRARY}][${DirectX_${inc_var_name}_INCLUDE_DIR}]")
        endif ()
    endmacro ()

    find_winsdk_header  (DDRAW   ddraw.h)
    find_winsdk_library (DDRAW   ddraw)
    find_combined       (DDRAW   DDRAW DDRAW)

    if (CMAKE_GENERATOR_TOOLSET MATCHES "_xp$")
        set (WINDOWS_XP TRUE)
    endif ()

    if (WINDOWS_XP)
        # Windows 7 SDKs, used by XP toolset, do not include d3d.h
        find_dxsdk_header   (D3D     d3d.h)
    else ()
        find_winsdk_header  (D3D     d3d.h)
    endif ()
    find_combined       (D3D     D3D DDRAW)

    find_dxsdk_header   (D3DX    d3dx.h)
    find_combined       (D3DX    D3DX D3DX)

    find_dxsdk_header   (D3D8    d3d8.h)
    find_dxsdk_library  (D3D8    d3d8)
    find_combined       (D3D8    D3D8 D3D8)

    find_dxsdk_header   (D3DX8   d3dx8.h)
    find_dxsdk_library  (D3DX8   d3dx8)
    find_combined       (D3DX8   D3DX8 D3DX8)

    find_winsdk_header  (D3D9    d3d9.h)
    find_winsdk_library (D3D9    d3d9)
    find_combined       (D3D9    D3D9 D3D9)

    find_dxsdk_header   (D3DX9   d3dx9.h)
    find_dxsdk_library  (D3DX9   d3dx9)
    find_combined       (D3DX9   D3DX9 D3DX9)

    if (NOT WINDOWS_XP)
        find_winsdk_header  (DXGI    dxgi.h)
        find_winsdk_header  (DXGI1_2 dxgi1_2.h)
        find_winsdk_header  (DXGI1_3 dxgi1_3.h)
        find_winsdk_header  (DXGI1_4 dxgi1_4.h)
        find_winsdk_library (DXGI    dxgi)

        find_winsdk_header  (D3D10   d3d10.h)
        find_winsdk_library (D3D10   d3d10)
        find_combined       (D3D10   D3D10 D3D10)

        find_winsdk_header  (D3D10_1 d3d10_1.h)
        find_winsdk_library (D3D10_1 d3d10_1)
        find_combined       (D3D10_1 D3D10_1 D3D10_1)

        find_dxsdk_header   (D3DX10  d3dx10.h)
        find_dxsdk_library  (D3DX10  d3dx10)
        find_combined       (D3DX10  D3DX10 D3DX10)

        find_winsdk_header  (D3D11   d3d11.h)
        find_winsdk_library (D3D11   d3d11)
        find_combined       (D3D11   D3D11 D3D11)
        find_winsdk_header  (D3D11_1 d3d11_1.h)
        find_combined       (D3D11_1 D3D11_1 D3D11)
        find_winsdk_header  (D3D11_2 d3d11_2.h)
        find_combined       (D3D11_2 D3D11_2 D3D11)
        find_winsdk_header  (D3D11_3 d3d11_3.h)
        find_combined       (D3D11_3 D3D11_3 D3D11)

        find_dxsdk_header   (D3DX11  d3dx11.h)
        find_dxsdk_library  (D3DX11  d3dx11)
        find_combined       (D3DX11  D3DX11 D3DX11)

        find_winsdk_header  (D2D1    d2d1.h)
        find_winsdk_library (D2D1    d2d1)
        find_combined       (D2D1    D2D1 D2D1)
        find_winsdk_header  (D2D1_1  d2d1_1.h)
        find_combined       (D2D1_1  D2D1_1 D2D1)
    endif ()

    find_program (DirectX_FXC_EXECUTABLE fxc
            HINTS ${DirectX_BIN_SEARCH_PATH}
            DOC "Path to fxc.exe executable."
            )

endif ()]]


#[[
if (MYGUI_STANDALONE_BUILD)
    SET(DirectX_INCLUDE_DIR "C:\MYGUIHACK DXSDK_DIR MYGUIBRACKETHACK/include" CACHE STRING "")
    SET(DirectX_LIBRARIES "d3d9.lib d3dx9.lib DxErr9.lib dxguid.lib" CACHE STRING "")
    SET(DIRECTX_LIB_DIR "C:\MYGUIHACK DXSDK_DIR MYGUIBRACKETHACK/lib" CACHE STRING "")
    SET(DirectX_FOUND TRUE)
else()
    IF (NOT DIRECTX_DIR)
        set(DIRECTX_DIR "" CACHE PATH "Path to DirectX SDK (set it if you don't have DIrectX SDK properly installed or CMake can't find path to it)")
    ENDIF ()

    if(WIN32) # The only platform it makes sense to check for DirectX SDK

        include(cmake/FindPkgMacros.cmake)
        findpkg_begin(DirectX)

        # Get path, convert backslashes as ${ENV_DXSDK_DIR}
        getenv_path(DXSDK_DIR)

        # construct search paths
        set(DirectX_PREFIX_PATH
                "${DIRECTX_DIR}"
                "${DXSDK_DIR}"
                "${ENV_DXSDK_DIR}"
                "C:/apps_x86/Microsoft DirectX SDK*"
                "C:/Program Files (x86)/Microsoft DirectX SDK*"
                "C:/apps/Microsoft DirectX SDK*"
                "C:/Program Files/Microsoft DirectX SDK*"
                "$ENV{ProgramFiles}/Microsoft DirectX SDK*"
                )
        create_search_paths(DirectX)
        # redo search if prefix path changed
        clear_if_changed(DirectX_PREFIX_PATH
                DirectX_LIBRARY
                DirectX_INCLUDE_DIR
                )

        find_path(DirectX_INCLUDE_DIR NAMES d3d9.h HINTS ${DirectX_INC_SEARCH_PATH})
        # dlls are in DIRECTX_DIR/Developer Runtime/x64|x86
        # lib files are in DIRECTX_DIR/Lib/x64|x86
        if(CMAKE_CL_64)
            set(DirectX_LIBPATH_SUFFIX "x64")
        else(CMAKE_CL_64)
            set(DirectX_LIBPATH_SUFFIX "x86")
        endif(CMAKE_CL_64)
        find_library(DirectX_LIBRARY NAMES d3d9 HINTS ${DirectX_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX_LIBPATH_SUFFIX})
        find_library(DirectX_D3DX9_LIBRARY NAMES d3dx9 HINTS ${DirectX_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX_LIBPATH_SUFFIX})
        find_library(DirectX_DXERR9_LIBRARY NAMES dxerr9 HINTS ${DirectX_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX_LIBPATH_SUFFIX})
        find_library(DirectX_DXGUID_LIBRARY NAMES dxguid HINTS ${DirectX_LIB_SEARCH_PATH} PATH_SUFFIXES ${DirectX_LIBPATH_SUFFIX})

        if(DirectX_INCLUDE_DIR)
            if (NOT DIRECTX_DIR)
                set(DIRECTX_DIR ${DirectX_INCLUDE_DIR}/.. CACHE PATH "Path to DirectX SDK (set it if you don't have DIrectX SDK properly installed or CMake can't find path to it)" FORCE)
            endif ()
        endif(DirectX_INCLUDE_DIR)

        findpkg_finish(DirectX)
        set(DirectX_LIBRARIES ${DirectX_LIBRARIES}
                ${DirectX_D3DX9_LIBRARY}
                ${DirectX_DXERR9_LIBRARY}
                ${DirectX_DXGUID_LIBRARY}
                )

        # look for D3D10.1 components
        if (DirectX_FOUND)
            find_path(DirectX_D3D10_INCLUDE_DIR NAMES d3d10_1shader.h HINTS ${DirectX_INCLUDE_DIR} NO_DEFAULT_PATH)
            get_filename_component(DirectX_LIBRARY_DIR "${DirectX_LIBRARY}" PATH)
            message(STATUS "DX lib dir: ${DirectX_LIBRARY_DIR}")
            find_library(DirectX_D3D10_LIBRARY NAMES d3d10 HINTS ${DirectX_LIBRARY_DIR} NO_DEFAULT_PATH)
            if (DirectX_D3D10_INCLUDE_DIR AND DirectX_D3D10_LIBRARY)
                set(DirectX_D3D10_FOUND TRUE)
                set(DirectX_D3D10_INCLUDE_DIRS ${DirectX_D3D10_INCLUDE_DIR})
                set(DirectX_D3D10_LIBRARIES ${DirectX_D3D10_LIBRARY})
            endif ()
        endif ()
    endif(WIN32)
endif ()

]]
