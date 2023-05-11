add_executable(
    blinky
    blinky.c
)

target_include_directories(
    blinky
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    blinky
    PRIVATE
    ${LIB_NAME}
)