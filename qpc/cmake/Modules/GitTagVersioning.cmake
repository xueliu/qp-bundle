# -- Minimum required version
cmake_minimum_required (VERSION 3.5.0)

# 
# Create version and short version information
# based on latest git-tagged version.
#
# Generated variables are:
# ${CMAKE_PROJECT_NAME}_VERSION
# This variable contains the latest tag version,
# the count of commits in current branch since tagging (if any),
# the short commit hash of current version (for count>0).
# In case of detected local changes in repository, the
# "-dirty" suffix will be appended to mark untracked changes.
#
# ${CMAKE_PROJECT_NAME}_VERSION_SHORT
# This variable contains the latest tag version as pure numerical
# string (usually in format major.minor.patch), if the git tagging
# is applied consequently. All chars beyond 0-9 and . will be removed.
#
# ${CMAKE_PROJECT_NAME}_VERSION_BRANCH
# This variable equals to ${CMAKE_PROJECT_NAME}_VERSION, 
# but offers an additional git branch information as
# a simple suffixed "-<branchname>".

# -- module version (of this .cmake file)
set(GIT_TAG_VERSIONING_MODULE_VERSION 1.0.0)

# -- Versioning with git tag
message(STATUS "GitTagVersioning: Version ${GIT_TAG_VERSIONING_MODULE_VERSION}")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
	# -- get git branch name
	execute_process(
		COMMAND
			git rev-parse --abbrev-ref HEAD
			WORKING_DIRECTORY
			"${CMAKE_CURRENT_SOURCE_DIR}"
			OUTPUT_VARIABLE
			${CMAKE_PROJECT_NAME}_BRANCH
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(${CMAKE_PROJECT_NAME}_BRANCH STREQUAL "")
		set(${CMAKE_PROJECT_NAME}_BRANCH unknown)
	endif()

	message(STATUS "  Git branch: ${${CMAKE_PROJECT_NAME}_BRANCH}")
	# -- get git full version info including possible dirty flag
	execute_process(
		COMMAND
			git describe --tags --always --dirty
			WORKING_DIRECTORY
			"${CMAKE_CURRENT_SOURCE_DIR}"
			OUTPUT_VARIABLE
			${CMAKE_PROJECT_NAME}_VERSION
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if(${CMAKE_PROJECT_NAME}_VERSION STREQUAL "")
		set(${CMAKE_PROJECT_NAME}_VERSION 0.0.0)
	endif()

	message(STATUS "  Git full version: ${${CMAKE_PROJECT_NAME}_VERSION}")
	# -- set variable including full version and branch name
	set(${CMAKE_PROJECT_NAME}_VERSION_BRANCH "${${CMAKE_PROJECT_NAME}_VERSION}-${${CMAKE_PROJECT_NAME}_BRANCH}")

	message(STATUS "  Git full version with branch name: ${${CMAKE_PROJECT_NAME}_VERSION_BRANCH}")
	# -- extract pure numerical (including dots) version info
	execute_process(
		COMMAND
			/bin/bash -c "git describe --tags --abbrev=0 | tr --delete --complement '0-9.'"
			WORKING_DIRECTORY
			"${CMAKE_CURRENT_SOURCE_DIR}"
			OUTPUT_VARIABLE
			${CMAKE_PROJECT_NAME}_VERSION_SHORT
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(${CMAKE_PROJECT_NAME}_VERSION_SHORT STREQUAL "")
		set(${CMAKE_PROJECT_NAME}_VERSION_SHORT 0.0.0)
	endif()
	message(STATUS "Git version: ${${CMAKE_PROJECT_NAME}_VERSION_SHORT}")
else()
	set(${CMAKE_PROJECT_NAME}_VERSION_SHORT 0.0.0)
	set(${CMAKE_PROJECT_NAME}_VERSION 0.0.0)
endif()
