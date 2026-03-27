find_package(Threads REQUIRED)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE THIRD_SRC ${CMAKE_SOURCE_DIR}/thirdparty/*.cpp)

add_executable(mytestserver ${SRC_FILES} ${THIRD_SRC})

target_include_directories(mytestserver PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(mytestserver PUBLIC mymuduo)
target_link_libraries(mytestserver PUBLIC Threads::Threads)


