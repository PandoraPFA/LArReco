cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(LArReco VERSION 05.01.00 LANGUAGES CXX)

# Automatically add the path of the linked libraries to the RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Apply the RPATH settings to the executable in the build tree, not just the installed one
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Allow relocatable installations
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

# Enforce out-of-source builds, a good practice from the original file 
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "LArReco requires an out-of-source build.")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path for ${PROJECT_NAME}" FORCE)
endif()

# Build Options
option(PANDORA_LIBTORCH "Build with LibTorch-dependent libraries" OFF)
option(PANDORA_MONITORING "Build with PandoraMonitoring support" ON)
option(LArReco_BUILD_DOCS "Build documentation for ${PROJECT_NAME}" OFF)

# Dependencies
if (NOT TARGET PandoraPFA::PandoraSDK)
    find_package(PandoraSDK 05.00.00 REQUIRED)
endif()
if (NOT TARGET PandoraPFA::LArContent)
    find_package(LArContent 05.00.00 REQUIRED)
endif()

if(PANDORA_LIBTORCH)
    find_package(LArDLContent 05.00.00 REQUIRED)
endif()

if(PANDORA_MONITORING)
    if (NOT TARGET PandoraPFA::PandoraMonitoring)
        find_package(PandoraMonitoring 05.00.00 REQUIRED)
    endif()
    find_package(ROOT 6.18.04 REQUIRED COMPONENTS Eve Geom RGL EG)
endif()

# --- Executable ---
add_executable(PandoraInterface test/PandoraInterface.cxx)

target_include_directories(PandoraInterface PRIVATE ${PROJECT_SOURCE_DIR}/include)

set_target_properties(PandoraInterface PROPERTIES CXX_STANDARD 17)
set_target_properties(PandoraInterface PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_options(PandoraInterface PRIVATE
    -Wall
    -Wextra
    -Werror
    -pedantic
    -Wno-long-long
    -Wno-sign-compare
    -Wshadow
    -fno-strict-aliasing
)

target_link_libraries(PandoraInterface PRIVATE
    PandoraPFA::PandoraSDK
    PandoraPFA::LArContent
)

if(PANDORA_LIBTORCH)
    target_link_libraries(PandoraInterface PRIVATE PandoraPFA::LArDLContent)
    target_compile_definitions(PandoraInterface PRIVATE -DLIBTORCH_DL=1)
endif()

if(PANDORA_MONITORING)
    target_link_libraries(PandoraInterface PRIVATE
        PandoraPFA::PandoraMonitoring
        ROOT::Eve
        ROOT::Geom
        ROOT::RGL
        ROOT::EG
    )
    target_compile_definitions(PandoraInterface PRIVATE -DMONITORING)
endif()

# Optional documents
if(LArReco_BUILD_DOCS)
    add_subdirectory(doc)
endif()

# Installation
install(DIRECTORY include/ DESTINATION include COMPONENT Development FILES_MATCHING PATTERN "*.h")
install(TARGETS PandoraInterface DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
