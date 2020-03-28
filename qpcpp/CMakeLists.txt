
set(POSIX_QV "qpcpp-posix-qv")
set(POSIX "qpcpp-posix")
set(POSIX_QUTEST "qpcpp-posix-qutest")

# install files while keeping folder structure
macro(INSTALL_WITH_HIERARCHY FILE_LIST DEST)
	foreach(F ${FILE_LIST})
		get_filename_component(F_DIR ${F} DIRECTORY)
		string(REPLACE ${CMAKE_CURRENT_LIST_DIR} "" DIR_REL ${F_DIR})
		install(FILES ${F} DESTINATION ${DEST}${DIR_REL}/)
	endforeach(F)
endmacro(INSTALL_WITH_HIERARCHY)

set (QPCPP_PUBLIC_HEADERS
	${CMAKE_CURRENT_LIST_DIR}/include/qassert.h
	${CMAKE_CURRENT_LIST_DIR}/include/qep.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qequeue.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qf.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qk.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qmpool.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qpcpp.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qpset.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qs_dummy.hpp
	${CMAKE_CURRENT_LIST_DIR}/include/qv.hpp
)

add_library(${POSIX_QV} "")
add_library(qpcpp::${POSIX_QV} ALIAS ${POSIX_QV})

target_compile_options(
	${POSIX_QV}
	PRIVATE
		-c
		-g
		-O
	#	-fno-pie
		-pedantic
		-Wall
		-Wextra
	#	-fno-rtti # we need rtti
		-fno-exceptions
		-fPIC # is it necessary
)

target_compile_definitions(${POSIX_QV} PUBLIC QP_API_VERSION=9999)

target_sources(${POSIX_QV}
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qep_hsm.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qep_msm.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_act.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_actq.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_defer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_dyn.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_mem.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_ps.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_qact.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_qeq.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_qmact.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_time.cpp

	${CMAKE_CURRENT_LIST_DIR}/ports/posix-qv/qf_port.cpp
	${CMAKE_CURRENT_LIST_DIR}/include/qstamp.cpp
)

target_include_directories(${POSIX_QV}
PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ports/posix-qv>
	$<INSTALL_INTERFACE:include>
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(${POSIX_QV} PROPERTIES VERSION ${${CMAKE_PROJECT_NAME}_VERSION_SHORT})
set_target_properties(${POSIX_QV} PROPERTIES SOVERSION ${${CMAKE_PROJECT_NAME}_VERSION_SHORT})
set_target_properties(${POSIX_QV}
	PROPERTIES
		PUBLIC_HEADER
		"${QPCPP_PUBLIC_HEADERS}"
)

# add the install targets
install (
	TARGETS		${POSIX_QV}
	EXPORT		${POSIX_QV}_targets
  	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT shlib
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${POSIX_QV}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


# -- write cmake package config file
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${POSIX_QV}/${POSIX_QV}-config-version.cmake"
	VERSION ${CMAKE_PROJECT_NAME}_VERSION_SHORT
	COMPATIBILITY AnyNewerVersion
)

export(EXPORT ${POSIX_QV}_targets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/${POSIX_QV}/${POSIX_QV}-targets.cmake"
	NAMESPACE qpcpp::
)

configure_file(cmake/${POSIX_QV}-config.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/${POSIX_QV}/${POSIX_QV}-config.cmake"
	COPYONLY
)

set(ConfigPackageLocation lib/cmake/${POSIX_QV})

install(EXPORT ${POSIX_QV}_targets
	FILE ${POSIX_QV}-targets.cmake
	NAMESPACE qpcpp::
	DESTINATION ${ConfigPackageLocation}
)

install(
	FILES cmake/${POSIX_QV}-config.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/${POSIX_QV}/${POSIX_QV}-config-version.cmake"
	DESTINATION ${ConfigPackageLocation}
	COMPONENT Devel
)

add_library(${POSIX} "")
add_library(qpcpp::${POSIX} ALIAS ${POSIX})

target_compile_options(
	${POSIX}
	PRIVATE
		-c
		-g
		-O
	#	-fno-pie
		-pedantic
		-Wall
		-Wextra
	#	-fno-rtti # we need rtti
		-fno-exceptions
		-fPIC # is it necessary
)

target_compile_definitions(${POSIX} PUBLIC QP_API_VERSION=9999)

target_sources(${POSIX}
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qep_hsm.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qep_msm.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_act.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_actq.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_defer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_dyn.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_mem.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_ps.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_qact.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_qeq.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_qmact.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/qf/qf_time.cpp

	${CMAKE_CURRENT_LIST_DIR}/ports/posix/qf_port.cpp
	${CMAKE_CURRENT_LIST_DIR}/include/qstamp.cpp
)

target_include_directories(${POSIX}
PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ports/posix>
	$<INSTALL_INTERFACE:include>
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(${POSIX} PROPERTIES VERSION ${${CMAKE_PROJECT_NAME}_VERSION_SHORT})
set_target_properties(${POSIX} PROPERTIES SOVERSION ${${CMAKE_PROJECT_NAME}_VERSION_SHORT})
set_target_properties(${POSIX}
	PROPERTIES
		PUBLIC_HEADER
		"${QPCPP_PUBLIC_HEADERS}"
)

# add the install targets
install (
	TARGETS		${POSIX}
	EXPORT		${POSIX}_targets
  	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT shlib
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${POSIX}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# -- write cmake package config file
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${POSIX}/${POSIX}-config-version.cmake"
	VERSION ${CMAKE_PROJECT_NAME}_VERSION_SHORT
	COMPATIBILITY AnyNewerVersion
)

export(EXPORT ${POSIX}_targets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/${POSIX}/${POSIX}-targets.cmake"
	NAMESPACE qp::
)

configure_file(cmake/${POSIX}-config.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/${POSIX}/${POSIX}-config.cmake"
	COPYONLY
)

set(ConfigPackageLocation lib/cmake/${POSIX})

install(EXPORT ${POSIX}_targets
	FILE ${POSIX}-targets.cmake
	NAMESPACE qp::
	DESTINATION ${ConfigPackageLocation}
)

install(
	FILES cmake/${POSIX}-config.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/${POSIX}/${POSIX}-config-version.cmake"
	DESTINATION ${ConfigPackageLocation}
	COMPONENT Devel
)
