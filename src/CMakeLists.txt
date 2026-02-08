find_package(Threads REQUIRED)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE THIRD_SRC ${CMAKE_SOURCE_DIR}/thirdparty/*.cpp)

add_executable(uchatserver ${SRC_FILES} ${THIRD_SRC})

target_include_directories(uchatserver PUBLIC
    ${CMAKE_SOURCE_DIR}/mymuduo
    ${CMAKE_SOURCE_DIR}/thirdparty
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(uchatserver PUBLIC mymuduo)
target_link_libraries(uchatserver PUBLIC Threads::Threads)


