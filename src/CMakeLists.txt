file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(mytestserver ${SRC_FILES})

target_include_directories(mytestserver PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/db
    ${CMAKE_SOURCE_DIR}/mymuduo
)

target_link_libraries(mytestserver PRIVATE mymuduo)
target_link_libraries(mytestserver PRIVATE db)
target_link_libraries(mytestserver PRIVATE crypto)


