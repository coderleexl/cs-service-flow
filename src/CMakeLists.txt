file (GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/src/*h")
file (GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*cpp")

add_library(${PROJECT_NAME} SHARED
    ${HEADERS}
    ${SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SERVICE_FLOW_LIBRARY
    $<$<CONFIG:Release>:NDEBUG>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../third_party/include>
)

