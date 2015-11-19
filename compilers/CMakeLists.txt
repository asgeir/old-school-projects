cmake_minimum_required(VERSION 2.8)
project(impasse)

find_package(FLEX REQUIRED)
find_package(Boost REQUIRED)

flex_target(impasse impasse.l ${CMAKE_CURRENT_BINARY_DIR}/impasse.lexer.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCES
	Ast.cpp
    Lexer.cpp
    main.cpp
    Parser.cpp
    TacBuilder.cpp
    Token.cpp
    ${FLEX_impasse_OUTPUTS})

add_executable(impasse ${SOURCES})

message("You will need to comment the following line out of impasse.lexer.cpp:")
message("    #define yyFlexLexer yyFlexLexer")
