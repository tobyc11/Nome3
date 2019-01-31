add_library(glad src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(glad ${DEFAULT_COMPILE_OPTIONS})
