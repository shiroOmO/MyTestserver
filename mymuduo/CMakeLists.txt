find_package(Threads REQUIRED)

file(GLOB_RECURSE MYMUDUO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(mymuduo STATIC ${MYMUDUO_SRC})

target_include_directories(mymuduo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mymuduo PUBLIC Threads::Threads)

