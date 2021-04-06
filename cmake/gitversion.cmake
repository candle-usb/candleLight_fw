# source: adapted from
# https://www.mattkeeter.com/blog/2018-01-06-versioning/

find_package(Git)

# quoting in execute_process is fragile... cmake encloses every arg with ' ' , but adding any kind of quoting to
# e.g. add spaces in the format string, will not work easily.

if (Git_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:fw_%h_%as -n 1
		OUTPUT_VARIABLE GIT_REV
		ERROR_QUIET
		WORKING_DIRECTORY ${SRCDIR}
	)
endif()

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "N/A")
    set(GIT_DIFF "")
    set(GIT_TAG "N/A")
else()
    execute_process(
		COMMAND ${GIT_EXECUTABLE} diff --quiet --exit-code
		RESULT_VARIABLE GIT_DIRTY
		WORKING_DIRECTORY ${SRCDIR})
    execute_process(
        COMMAND git describe --exact-match --tags
        OUTPUT_VARIABLE GIT_TAG
		ERROR_QUIET
		WORKING_DIRECTORY ${SRCDIR})
	if (NOT "${GIT_DIRTY}" EQUAL 0)
		#append '+' if unclean tree
		set(GIT_DIFF "+")
	endif()

    string(STRIP "${GIT_REV}" GIT_REV)
endif()

set(VERSION "#define GIT_HASH \"${GIT_REV}${GIT_DIFF}\"\n")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.h)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.h VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version.h "${VERSION}")
endif()
