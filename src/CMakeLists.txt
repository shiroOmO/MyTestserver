find_package(Threads REQUIRED)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(mytestserver ${SRC_FILES})

target_include_directories(mytestserver PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/db
    ${CMAKE_SOURCE_DIR}/mymuduo
)

target_compile_definitions(mytestserver PRIVATE CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(mytestserver PUBLIC mymuduo)
target_link_libraries(mytestserver PUBLIC db)
target_link_libraries(mytestserver PUBLIC Threads::Threads)
target_link_libraries(mytestserver PUBLIC mysqlclient)
target_link_libraries(mytestserver PUBLIC crypto)


