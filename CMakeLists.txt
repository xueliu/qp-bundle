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

# -- set cxx 14 standard default
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -- set c 99 standard default
set(CMAKE_C_STANDARD 14)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -- Required to build
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
