set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


cmake_minimum_required(VERSION 3.20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

project(cis565_project5_vulkan_grass_rendering)

OPTION(USE_D2D_WSI "Build the project using Direct to Display swapchain" OFF)

find_package(Vulkan REQUIRED)

IF(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSE(WIN32)
    find_package(Threads REQUIRED)
IF(USE_D2D_WSI)
    MESSAGE("Using direct to display extension...")
    add_definitions(-D_DIRECT2DISPLAY)
ELSE(USE_D2D_WSI)
    find_package(XCB REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
ENDIF(USE_D2D_WSI)
    # Todo : android?
ENDIF(WIN32)

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 11)

# Enable the creation of folders for Visual Studio projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(ExternalTarget folder target)
  set_property(TARGET ${target} PROPERTY FOLDER ${folder})
endfunction(ExternalTarget)

function(InternalTarget folder target)
  ExternalTarget("${folder}" ${target})
  if (MSVC)
    get_target_property(targetSources ${target} SOURCES)
    foreach(sourceFile IN ITEMS ${targetSources})
        if (IS_ABSOLUTE "${sourceFile}")
          file(RELATIVE_PATH sourceFile "${CMAKE_CURRENT_SOURCE_DIR}" "${sourceFile}")
        endif()
        get_filename_component(sourceDir "${sourceFile}" PATH)
        string(REPLACE "/" "\\" sourceDir "${sourceDir}")
        source_group("${sourceDir}" FILES "${sourceFile}")
    endforeach()
  endif()
endfunction(InternalTarget)

# Compiler specific stuff
IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

IF(WIN32)
  # Nothing here (yet)
ELSE(WIN32)
    link_libraries(${XCB_LIBRARIES} ${VULKAN_LIB})
ENDIF(WIN32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

add_subdirectory(external)
add_subdirectory(src)
