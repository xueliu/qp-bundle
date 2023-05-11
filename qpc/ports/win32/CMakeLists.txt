
# install files while keeping folder structure
macro(INSTALL_WITH_HIERARCHY FILE_LIST DEST)
    foreach(F ${FILE_LIST})
        get_filename_component(F_DIR ${F} DIRECTORY)
        string(REPLACE ${CMAKE_CURRENT_LIST_DIR} "" DIR_REL ${F_DIR})
        install(FILES ${F} DESTINATION ${DEST}${DIR_REL}/)
    endforeach(F)
endmacro(INSTALL_WITH_HIERARCHY)

add_library(${LIB_NAME} "")
add_library(qpc::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_options(
    ${LIB_NAME}
    PRIVATE
    -c
    -fno-pie
    -pedantic
    -Wall
    -Wextra
    -W
    $<$<CONFIG:Debug>:-DNDEBUG>
)

target_compile_definitions(${LIB_NAME} PUBLIC QP_API_VERSION=9999)

target_sources(
    ${LIB_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/qf_port.c
    ${QEP_SOURCES}
    ${QF_SOURCES}
    ${QPC_SOURCES}
)

target_include_directories(
    ${LIB_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

target_link_libraries(
    ${LIB_NAME}
    PUBLIC
#    Threads::Threads
)
