file (GLOB_RECURSE EXAMPLE_HEADERS "${PROJECT_SOURCE_DIR}/examples/*h")
file (GLOB_RECURSE EXAMPLE_SOURCES "${PROJECT_SOURCE_DIR}/examples/*cpp")

add_executable(test ${EXAMPLE_HEADERS} ${EXAMPLE_SOURCES})

target_link_libraries(${PROJECT_NAME}  gtest gtest_main)

target_link_libraries(test PRIVATE ${PROJECT_NAME})
target_include_directories(test INTERFACE ${PROJECT_NAME})