cmake_minimum_required(VERSION 3.10)
project(Second_draft_of_tictactoe)
set(CMAKE_CXX_STANDARD 17)

# Include headers
include_directories(include)
include_directories(googletest/googletest/include)

# Add Google Test
add_subdirectory(googletest)

# Test executable
add_executable(test_game
  tests/test_game.cpp
  src/tictactoe.cpp 
)

# Link with GTest
target_include_directories(test_game PRIVATE include)
target_link_libraries(test_game gtest)
