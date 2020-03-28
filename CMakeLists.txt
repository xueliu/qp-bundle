# -- Minimum required version
cmake_minimum_required (VERSION 3.5)

# -- Project name
project (qp-bundle)

# -- Various includes
include (CTest)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include (GNUInstallDirs)
include (CMakePackageConfigHelpers)

# -- Enable Git Tag based versioning
include (GitTagVersioning)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
if (NOT (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr" ) )
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()

# -- Set cxx 14 standard default
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -- Set c 99 standard default
set(CMAKE_C_STANDARD 14)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -- Required to build
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# -- https://blog.kitware.com/cmake-and-the-default-build-type/
# -- Set a default build type if none was specified
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()
 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
# -- Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# -- Options for shared lib and strict cxx flags (defaults on)
#option(qp-bundle_USE_STRICT_CXX_FLAGS "Use strict CXX Flags on compile" ON)
option(qp-bundle_build_shared_libs "build shared libraries" OFF)
set(BUILD_SHARED_LIBS ${qp-bundle_build_shared_libs})

# -- Add qpcpp
add_subdirectory(qpcpp)
