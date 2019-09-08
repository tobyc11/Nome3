set(CMAKE_CXX_STANDARD 17)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY Binaries)

option(NOME_BUILD_TESTS "Build tests" OFF)

file(GLOB PG2_SOURCES
    *.h *.cpp
    Flow/*.h Flow/*.cpp
    FlowViz/*.h FlowViz/*.cpp
    Parsing/*.h Parsing/*.cpp Parsing/*.l Parsing/*.y
    QtFrontend/*.h QtFrontend/*.cpp QtFrontend/*.ui
    Scene/*.h Scene/*.cpp
)

file(GLOB PG2_TEST_SOURCES
    Tests/*.h Tests/*.cpp
)

find_package(BISON 3.4)
find_package(FLEX 2.6)

BISON_TARGET(NomeParser Parsing/NomeParser.y ${CMAKE_CURRENT_BINARY_DIR}/NomeParser.cpp)
FLEX_TARGET(NomeScanner Parsing/NomeLexer.l ${CMAKE_CURRENT_BINARY_DIR}/NomeLexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(NomeScanner NomeParser)

set(PG2_GENERATED_SOURCES
    ${BISON_NomeParser_OUTPUTS}
    ${FLEX_NomeScanner_OUTPUTS}
)

macro(config_playground2 target)
    target_link_libraries(${target} PRIVATE Foundation)
    target_link_libraries(${target} PRIVATE Math)
    target_compile_options(${target} ${DEFAULT_COMPILE_OPTIONS})

    find_package(Qt5Widgets CONFIG REQUIRED)
    target_link_libraries(${target} PRIVATE Qt5::Widgets)

    find_package(OpenMesh REQUIRED)
    target_include_directories(${target} PRIVATE ${OPENMESH_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE ${OPENMESH_LIBRARIES})
endmacro()

add_executable(Nome3 ${PG2_SOURCES} ${PG2_GENERATED_SOURCES})
config_playground2(Nome3)
if(NOME_BUILD_TESTS)
    add_executable(Nome3_test ${PG2_SOURCES} ${PG2_GENERATED_SOURCES} ${PG2_TEST_SOURCES})
    config_playground2(Nome3_test)
    target_compile_definitions(Nome3_test PRIVATE DISABLE_MAIN_FOR_TESTS)
endif()

#Attempt to windeployqt
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET Nome3 POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:Nome3>
    )
endif()

install(DIRECTORY Resources DESTINATION .)
install(DIRECTORY $<TARGET_FILE_DIR:Nome3>/ DESTINATION .)
