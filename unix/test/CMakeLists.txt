cmake_minimum_required(VERSION 3.10.0)

project(lemon_watcher_unix_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    add_executable(watcher_unix ${CMAKE_CURRENT_SOURCE_DIR}/../watcher_macos.mm)
else()
    add_executable(watcher_unix ${CMAKE_CURRENT_SOURCE_DIR}/../watcher_linux.cpp)
endif()

add_executable(hello hello.c)
add_executable(mle mle.c)
file(COPY hello.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
add_test(NAME watcher_run_c_test COMMAND python3 scripts/run.py)
add_test(NAME watcher_run_sh_test COMMAND python3 scripts/run_sh.py)
add_test(NAME watcher_mle_test COMMAND python3 scripts/mle.py)