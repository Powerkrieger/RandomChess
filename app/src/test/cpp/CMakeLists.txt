# Host-side tests for the platform-independent game logic in app/src/main/cpp/chess.
# They don't need the NDK: build and run with any C++17 compiler, e.g.
#
#   cmake -S app/src/test/cpp -B build/cpp-tests
#   cmake --build build/cpp-tests
#   ctest --test-dir build/cpp-tests --output-on-failure

cmake_minimum_required(VERSION 3.22)
project(randomchess-tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(GAME_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../main/cpp)

add_library(chess STATIC
    ${GAME_SRC}/chess/Piece.cpp
    ${GAME_SRC}/chess/Pieces.cpp
    ${GAME_SRC}/chess/Board.cpp
    ${GAME_SRC}/chess/Game.cpp
    ${GAME_SRC}/chess/Rules.cpp
    ${GAME_SRC}/chess/ShogunAi.cpp
    ${GAME_SRC}/Stats.cpp)
target_include_directories(chess PUBLIC ${GAME_SRC})
target_compile_options(chess PRIVATE -Wall -Wextra)

enable_testing()

foreach (test chess_rules shogun_rules shogun_ai stats)
    add_executable(${test}_test ${test}_test.cpp)
    target_link_libraries(${test}_test PRIVATE chess)
    target_compile_options(${test}_test PRIVATE -Wall -Wextra)
    add_test(NAME ${test} COMMAND ${test}_test)
endforeach ()
