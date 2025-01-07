# src/CMakeLists.txt

# Collect source files
set(CORE_SOURCES
    core/error_reporter.cpp
    core/utils.cpp
)

set(LEXER_SOURCES
    lexer/lexer.cpp
    lexer/token.cpp
)

set(PARSER_SOURCES
    parser/parser.cpp
)

# Create the compiler executable
add_executable(tspp-compiler
    main.cpp
    ${CORE_SOURCES}
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
)

# Add include directories
target_include_directories(tspp-compiler 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compiler flags
if(MSVC)
    target_compile_options(tspp-compiler PRIVATE /W4)
else()
    target_compile_options(tspp-compiler PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install rules
install(TARGETS tspp-compiler
    RUNTIME DESTINATION bin
)