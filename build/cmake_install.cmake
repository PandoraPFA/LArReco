# Install script for directory: /storage/epp2/phupqr/Pandora.repos/Development/LArReco

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/storage/epp2/phupqr/Pandora.repos/Development/LArReco")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Runtime")
  FOREACH(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLArReco.so.03.16.00"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLArReco.so.03.16"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLArReco.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_CHECK
           FILE "${file}"
           RPATH "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/lib:/storage/epp2/phupqr/Pandora.repos/Development/PandoraPFA/lib")
    ENDIF()
  ENDFOREACH()
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/build/lib/libLArReco.so.03.16.00"
    "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/build/lib/libLArReco.so.03.16"
    "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/build/lib/libLArReco.so"
    )
  FOREACH(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLArReco.so.03.16.00"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLArReco.so.03.16"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLArReco.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/storage/epp2/phupqr/Pandora.repos/Development/PandoraPFA/lib:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
           NEW_RPATH "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/lib:/storage/epp2/phupqr/Pandora.repos/Development/PandoraPFA/lib")
      IF(CMAKE_INSTALL_DO_STRIP)
        EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "${file}")
      ENDIF(CMAKE_INSTALL_DO_STRIP)
    ENDIF()
  ENDFOREACH()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Runtime")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Development")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/include/" FILES_MATCHING REGEX "/[^/]*\\.h$")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Development")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/PandoraInterface" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/PandoraInterface")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/PandoraInterface"
         RPATH "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/lib:/storage/epp2/phupqr/Pandora.repos/Development/PandoraPFA/lib:/cvmfs/larsoft.opensciencegrid.org/products/root/v6_18_04d/Linux64bit+3.10-2.17-e19-prof/lib")
  ENDIF()
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE FILES "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/build/bin/PandoraInterface")
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/PandoraInterface" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/PandoraInterface")
    FILE(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/PandoraInterface"
         OLD_RPATH "/storage/epp2/phupqr/Pandora.repos/Development/PandoraPFA/lib:/cvmfs/larsoft.opensciencegrid.org/products/root/v6_18_04d/Linux64bit+3.10-2.17-e19-prof/lib:/storage/epp2/phupqr/Pandora.repos/Development/LArReco/build/lib:"
         NEW_RPATH "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/lib:/storage/epp2/phupqr/Pandora.repos/Development/PandoraPFA/lib:/cvmfs/larsoft.opensciencegrid.org/products/root/v6_18_04d/Linux64bit+3.10-2.17-e19-prof/lib")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/PandoraInterface")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/build/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/storage/epp2/phupqr/Pandora.repos/Development/LArReco/build/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
