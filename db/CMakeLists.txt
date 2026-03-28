file(GLOB_RECURSE DB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(db STATIC ${DB_SRC})

target_include_directories(db PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/mymuduo
)

target_link_libraries(db PUBLIC mysqlclient)
