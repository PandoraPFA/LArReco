########################################################
# cmake file for building LArReco
# @author Jan Engels, DESY
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2 FATAL_ERROR)
########################################################


# project name
PROJECT( LArReco )


# project version
SET( LArReco_VERSION_MAJOR 01 )
SET( LArReco_VERSION_MINOR 00 )
SET( LArReco_VERSION_PATCH 01 )


### CMAKE ###################################################################

INCLUDE( Default_settings )


### SOURCE ##################################################################

#include directories
INCLUDE_DIRECTORIES( ./include )

AUX_SOURCE_DIRECTORY( ./src sources )

# require proper C++
ADD_DEFINITIONS( "-Wall -ansi -pedantic" )

# need long long for int64 for now
ADD_DEFINITIONS( "-Wno-long-long -Wno-sign-compare -fno-strict-aliasing" )

INCLUDE( CheckCXXCompilerFlag )
IF( NOT CMAKE_CXX_FLAGS )
    SET( CMAKE_CXX_FLAGS "-std=c++11" )
    SET( PROCEED_WITHOUT_CXX_FLAGS ON )
ENDIF()
UNSET( COMPILER_SUPPORTS_FLAGS CACHE )
CHECK_CXX_COMPILER_FLAG( ${CMAKE_CXX_FLAGS} COMPILER_SUPPORTS_FLAGS )
IF( COMPILER_SUPPORTS_FLAGS )
    MESSAGE( STATUS "The compiler ${CMAKE_CXX_COMPILER} supports user-provided flags ${CMAKE_CXX_FLAGS}" )
    ADD_DEFINITIONS( ${CMAKE_CXX_FLAGS} )
ELSE()
    MESSAGE( STATUS "The compiler ${CMAKE_CXX_COMPILER} does not support user-provided flags ${CMAKE_CXX_FLAGS}" )
    IF( PROCEED_WITHOUT_CXX_FLAGS )
        MESSAGE( STATUS "Proceed without user-provided flags ${CMAKE_CXX_FLAGS}" )
        UNSET( CMAKE_CXX_FLAGS )
    ELSE()
        MESSAGE( FATAL_ERROR "CMake abort." )
    ENDIF()
ENDIF()


### DEPENDENCIES ############################################################

FIND_PACKAGE( LArContent 01.00.00 REQUIRED )
FIND_PACKAGE( PandoraSDK 01.00.00 REQUIRED )

FOREACH( pkg LArContent PandoraSDK )
    IF( ${pkg}_FOUND )
        INCLUDE_DIRECTORIES( ${${pkg}_INCLUDE_DIRS} )
        LINK_LIBRARIES( ${${pkg}_LIBRARIES} )
        ADD_DEFINITIONS ( ${${pkg}_DEFINITIONS} )
    ENDIF()
ENDFOREACH()

IF( PANDORA_MONITORING )
    LIST( APPEND CMAKE_MODULE_PATH "$ENV{ROOTSYS}/etc/cmake/" )
    ADD_DEFINITIONS( "-DMONITORING" )
    FIND_PACKAGE( PandoraMonitoring 01.00.00 REQUIRED )
    FIND_PACKAGE( ROOT 5.26.00 REQUIRED COMPONENTS Eve Geom RGL EG ) # minimum required ROOT version

    FOREACH( pkg PandoraMonitoring ROOT )
        IF( ${pkg}_FOUND )
            INCLUDE_DIRECTORIES( ${${pkg}_INCLUDE_DIRS} )
            LINK_LIBRARIES( ${${pkg}_LIBRARIES} )
            ADD_DEFINITIONS ( ${${pkg}_DEFINITIONS} )
        ENDIF()
    ENDFOREACH()
ENDIF()


### DOCUMENTATION ###########################################################

OPTION( INSTALL_DOC "Set to OFF to skip build/install Documentation" OFF )

IF( INSTALL_DOC AND EXISTS "${PROJECT_SOURCE_DIR}/doc/CMakeLists.txt" )
    ADD_SUBDIRECTORY( ./doc )
ELSE()
    MESSAGE( STATUS "INSTALL_DOC set to OFF" )
    SET( INSTALL_DOC OFF )
ENDIF()


### OUTPUT ##################################################################

# add library
ADD_SHARED_LIBRARY( ${PROJECT_NAME} ${sources} )

# make standalone executable
ADD_EXECUTABLE( PandoraInterface ${PROJECT_SOURCE_DIR}/test/${program}.cxx )
TARGET_LINK_LIBRARIES( PandoraInterface ${PROJECT_NAME} )


### INSTALL #################################################################

# install library
INSTALL_SHARED_LIBRARY( ${PROJECT_NAME} DESTINATION lib )

# install executable
INSTALL( TARGETS PandoraInterface DESTINATION bin PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE )

# display some variables and write them to cache
DISPLAY_STD_VARIABLES()
