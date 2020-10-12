# Setup ANTLR4 Runtime

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)
# add external build for antlrcpp
include(ExternalAntlr4Cpp)

# Generate Lexer and Parser with ANTLR
find_package(ANTLR REQUIRED)
antlr_target(NomGenerate Nom.g4 LEXER PARSER LISTENER VISITOR)

# Export the generated parser as a library
add_library(NomParsing STATIC ${ANTLR_NomGenerate_CXX_OUTPUTS})
target_include_directories(NomParsing PUBLIC ${ANTLR4_INCLUDE_DIRS})
target_compile_definitions(NomParsing PUBLIC ANTLR4CPP_STATIC)
target_link_libraries(NomParsing PUBLIC antlr4_static)
target_include_directories(NomParsing PUBLIC ${ANTLR_NomGenerate_OUTPUT_DIR})
