cmake_minimum_required(VERSION 3.16)
project(esp32_board_tests LANGUAGES CXX)
enable_testing()

# board.cpp only needs sdkconfig's board selection; no ESP-IDF runtime mocks.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sdkconfig.h" "// Board choice supplied by the test target.\n")
foreach(board IN ITEMS ESP32_CYD ESP32S3_BREADBOARD CROWPANEL_21)
    add_executable(board_${board} board_profiles.cpp ../main/board.cpp)
    target_compile_features(board_${board} PRIVATE cxx_std_17)
    target_include_directories(board_${board} PRIVATE ../main "${CMAKE_CURRENT_BINARY_DIR}")
    target_compile_definitions(board_${board} PRIVATE CONFIG_HG_BOARD_${board}=1)
    add_test(NAME board_${board} COMMAND board_${board})
endforeach()
