# src/CMakeLists.txt
add_library(tspp_lib
    lexer/lexer.cpp
    parser/parser.cpp
)

target_include_directories(tspp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(tspp main.cpp)
target_link_libraries(tspp PRIVATE tspp_lib)