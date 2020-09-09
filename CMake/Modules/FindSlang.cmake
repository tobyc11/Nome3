#Prepare
if(CMAKE_GENERATOR_PLATFORM MATCHES "^[Ww][Ii][Nn]32$")
    set(TARGET_ARCH x86)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Xx]64$")
    set(TARGET_ARCH x64)
else()
    if(CMAKE_GENERATOR MATCHES "Win64")
        set(TARGET_ARCH x64)
    else()
        set(TARGET_ARCH x86)
    endif()
endif()

#message(STATUS "Slang arch detected => ${TARGET_ARCH}")

#Find stuff
find_path(Slang_INCLUDE_DIR
  NAMES slang.h
)

find_library(Slang_LIBRARY_RELEASE
  NAMES slang
  PATH_SUFFIXES "bin/windows-${TARGET_ARCH}/release"
)

find_library(Slang_LIBRARY_DEBUG
  NAMES slang
  PATH_SUFFIXES "bin/windows-${TARGET_ARCH}/debug"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Slang
  FOUND_VAR Slang_FOUND
  REQUIRED_VARS
    Slang_LIBRARY_DEBUG
    Slang_LIBRARY_RELEASE
    Slang_INCLUDE_DIR
)

mark_as_advanced(
  Slang_LIBRARY_RELEASE
  Slang_LIBRARY_DEBUG
  Slang_INCLUDE_DIR
)

#Output
if(Slang_FOUND)
  if (NOT TARGET Slang::Slang)
    add_library(Slang::Slang UNKNOWN IMPORTED)
  endif()
  if (Slang_LIBRARY_RELEASE)
    set_property(TARGET Slang::Slang APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(Slang::Slang PROPERTIES
      IMPORTED_LOCATION_RELEASE "${Slang_LIBRARY_RELEASE}"
    )
  endif()
  if (Slang_LIBRARY_DEBUG)
    set_property(TARGET Slang::Slang APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(Slang::Slang PROPERTIES
      IMPORTED_LOCATION_DEBUG "${Slang_LIBRARY_DEBUG}"
    )
  endif()
  set_target_properties(Slang::Slang PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${Slang_INCLUDE_DIR}"
  )
endif()
