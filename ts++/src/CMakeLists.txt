add_library(core
    core/diagnostics/error_reporter.cpp
    core/utils/file_utils.cpp
    core/utils/log_utils.cpp
    core/utils/string_utils.cpp
)

add_library(tokens
    tokens/stream/token_stream.cpp
)

add_library(lexer
    lexer/lexer.cpp
    lexer/scanner/base/scanner_base.cpp
    lexer/scanner/specialized/identifier_scanner.cpp
    lexer/scanner/specialized/number_scanner.cpp
    lexer/scanner/specialized/operator_scanner.cpp
    lexer/scanner/specialized/string_scanner.cpp
    lexer/scanner/token_scanner.cpp
    lexer/state/lexer_state.cpp
)

add_library(parser
    parser/parser.cpp
    parser/visitors/parse_visitor/base/base_parse_visitor.cpp
    parser/visitors/parse_visitor/expression/expression_parse_visitor.cpp
    parser/visitors/parse_visitor/declaration/declaration_parse_visitor.cpp
    parser/visitors/parse_visitor/statement/statement_parse_visitor.cpp
    parser/visitors/type_check_visitor/resolved_type.cpp
    parser/visitors/type_check_visitor/type_check_visitor.cpp
    parser/visitors/type_check_visitor/type_scope.cpp
)

add_library(repl
    repl/repl.cpp
)

target_link_libraries(core PUBLIC tokens)
target_link_libraries(tokens PUBLIC core)
target_link_libraries(lexer PUBLIC core tokens)
target_link_libraries(parser PUBLIC core tokens)
target_link_libraries(repl PUBLIC core tokens lexer parser)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tokens PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(repl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(tspp main.cpp)

target_link_libraries(tspp
    PRIVATE
        core
        tokens
        lexer
        parser
        repl
)