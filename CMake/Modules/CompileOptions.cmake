#Adapted from https://github.com/cginternals/glbinding/blob/master/cmake/CompileOptions.cmake
#
#Copyright (c) 2014-2015 Computer Graphics Systems Group at the Hasso-Plattner-Institute and CG Internals GmbH, Germany. 
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

set(DEFAULT_COMPILE_OPTIONS)

# MSVC compiler options
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
        PRIVATE
        $<$<CONFIG:Release>: 
        /Gw           # -> whole program global optimization
        /GS-          # -> buffer security check: no 
        /GL           # -> whole program optimization: enable link-time code generation (disables Zi)
        /GF           # -> enable string pooling
        >
        PUBLIC
    )
endif()

# GCC and Clang compiler options
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
        PUBLIC
        $<$<PLATFORM_ID:Darwin>:
            -pthread
        >
    )
endif()

message(STATUS DEFAULT_COMPILE_OPTIONS=${DEFAULT_COMPILE_OPTIONS})
