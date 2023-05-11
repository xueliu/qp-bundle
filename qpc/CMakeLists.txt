# -- Minimum required version
cmake_minimum_required (VERSION 3.14)

# -- Project name
project (qpc)

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

# -- Set c 11 standard default
set(CMAKE_C_STANDARD 11)
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
#option(qpc_build_USE_STRICT_CXX_FLAGS "Use strict CXX Flags on compile" ON)
set(qpc_build_shared_libs "build shared libraries" OFF)
set(BUILD_SHARED_LIBS ${qpc_build_build_shared_libs})

set(QPC_PORT "win32" CACHE STRING "QPC Port")
set_property(CACHE QPC_PORT PROPERTY STRINGS "posix" "win32")

if( QPC_PORT STREQUAL "win32" OR QPC_PORT STREQUAL "posix")
    set(QPC_VARIANT "qv" CACHE STRING "QPC Variant")
    set_property(CACHE QPC_VARIANT PROPERTY STRINGS "none" "qutest" "qv")
endif()

message(STATUS "QPC Port: " ${QPC_PORT})
message(STATUS "QPC Variant: " ${QPC_VARIANT})

# -- Add qpc headers
file(GLOB QPC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/*.h)
file(GLOB QPC_SOURCES ${CMAKE_CURRENT_LIST_DIR}/include/*.c)

# -- Add qep qf and qk
include(src/CMakeLists.txt)

# -- Set active library name
if(QPC_VARIANT STREQUAL "none")
    set( LIB_NAME ${PROJECT_NAME}-${QPC_PORT} )
else ( )
    set( LIB_NAME ${PROJECT_NAME}-${QPC_PORT}-${QPC_VARIANT} )
endif ()
message(STATUS "QPC Library: " ${LIB_NAME})

# -- Import QPC library
if( QPC_PORT STREQUAL "win32" OR QPC_PORT STREQUAL "posix")
    if ( QPC_VARIANT STREQUAL "qv")
        # -- Add port win32-qv
        add_subdirectory(ports/win32-qv)
    else (  )
        # -- Add port win32
        add_subdirectory(ports/win32)
    endif()
endif()

# -- Import examples
if( QPC_PORT STREQUAL "win32" OR QPC_PORT STREQUAL "posix")
    add_subdirectory(examples/workstation)
endif (  )

