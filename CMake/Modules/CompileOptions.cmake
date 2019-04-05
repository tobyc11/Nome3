#Adapted from https://github.com/cginternals/glbinding/blob/master/cmake/CompileOptions.cmake
#
#Copyright (c) 2014-2015 Computer Graphics Systems Group at the Hasso-Plattner-Institute and CG Internals GmbH, Germany. 
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

set(DEFAULT_COMPILE_OPTIONS)

# MSVC compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
    PRIVATE
        /MP           # -> build with multiple processes
        /W4           # -> warning level 4
        # /WX         # -> treat warnings as errors
        /wd4251       # -> disable warning: 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
        /wd4275       # -> disable warning: non dll-interface class 'type' used as base for dll-interface class 'type2'
        /wd4592       # -> disable warning: 'identifier': symbol will be dynamically initialized (implementation limitation)
        # /wd4201     # -> disable warning: nonstandard extension used: nameless struct/union (caused by GLM)
        /wd4127       # -> disable warning: conditional expression is constant (caused by Qt)
		/wd4100       # -> disable warning: unused parameters

        # /Zm114      # -> Memory size for precompiled headers (insufficient for msvc 2013)
        # /Zm200      # -> Memory size for precompiled headers
        
        #$<$<CONFIG:Debug>:
        #/RTCc         # -> value is assigned to a smaller data type and results in a data loss
        #>

        $<$<CONFIG:Release>: 
        /Gw           # -> whole program global optimization
        /GS-          # -> buffer security check: no 
        /GL           # -> whole program optimization: enable link-time code generation (disables Zi)
        /GF           # -> enable string pooling
        >
        
        # No manual c++11 enable for MSVC as all supported MSVC versions for cmake-init have C++11 implicitly enabled (MSVC >=2013)

        PUBLIC
    )
endif ()

# GCC and Clang compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
    PRIVATE
        #-fno-exceptions # since we use stl and stl is intended to use exceptions, exceptions should not be disabled

        -Wall
        -Wextra
        -Wunused

        -Wreorder
        -Wignored-qualifiers
        -Wmissing-braces
        -Wreturn-type
        -Wswitch
        -Wswitch-default
        -Wuninitialized
        -Wmissing-field-initializers
        
        $<$<CXX_COMPILER_ID:GNU>:
            -Wmaybe-uninitialized
        
            -Wno-unknown-pragmas
            
            $<$<VERSION_GREATER:$<CXX_COMPILER_VERSION>,4.8>:
                -Wpedantic
                
                -Wreturn-local-addr
            >
        >
        
        $<$<CXX_COMPILER_ID:Clang>:
            -Wpedantic
                
            # -Wreturn-stack-address # gives false positives
        >
    PUBLIC
        $<$<PLATFORM_ID:Darwin>:
            -pthread
        >
    )
endif ()

message(STATUS DEFAULT_COMPILE_OPTIONS=${DEFAULT_COMPILE_OPTIONS})
