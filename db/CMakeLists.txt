cmake_minimum_required(VERSION 3.0)

add_library(db STATIC
    DbConnection.cpp
    UserDao.cpp
    MessageDao.cpp
)

target_include_directories(db PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/mymuduo
    /usr/include/mysql
)

target_link_libraries(db PUBLIC mysqlclient)
