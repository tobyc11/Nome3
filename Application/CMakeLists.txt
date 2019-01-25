set(CMAKE_CXX_STANDARD 17)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

file(GLOB PG2_SOURCES
    *.h *.cpp
    #DSLDemo/*.h DSLDemo/*.cpp
    Flow/*.h Flow/*.cpp
    FlowViz/*.h FlowViz/*.cpp
    #Nome/*.h Nome/*.cpp
    Parsing/*.h Parsing/*.cpp Parsing/*.l Parsing/*.y
    QtFrontend/*.h QtFrontend/*.cpp QtFrontend/*.ui
    Render/*.h Render/*.cpp
    Scene/*.h Scene/*.cpp
)

file(GLOB PG2_TEST_SOURCES
    Tests/*.h Tests/*.cpp
)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(NomeParser Parsing/NomeParser.y ${CMAKE_CURRENT_BINARY_DIR}/NomeParser.cpp)
FLEX_TARGET(NomeScanner Parsing/NomeLexer.l ${CMAKE_CURRENT_BINARY_DIR}/NomeLexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(NomeScanner NomeParser)

set(PG2_GENERATED_SOURCES
    ${BISON_NomeParser_OUTPUTS}
    ${FLEX_NomeScanner_OUTPUTS}
)

macro(config_playground2 target)
    target_link_libraries(${target} PRIVATE imgui)
    target_link_libraries(${target} PRIVATE glad)
    target_link_libraries(${target} PRIVATE Foundation)
    target_link_libraries(${target} PRIVATE Math)

    find_package(SDL2 REQUIRED)
    if(TARGET SDL2::SDL2)
        target_link_libraries(${target} PRIVATE SDL2::SDL2)
    else()
        target_include_directories(${target} PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(${target} PRIVATE ${SDL2_LIBRARIES})
    endif()

    find_package(Qt5Widgets CONFIG REQUIRED)
    target_link_libraries(${target} PRIVATE Qt5::Widgets)

    #find_package(Boost REQUIRED)
    #target_link_libraries(${target} PRIVATE Boost::boost)

    find_package(Lua REQUIRED)
    target_include_directories(${target} PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(${target} PRIVATE ${LUA_LIBRARIES})

	find_package(OpenMesh REQUIRED)
	target_include_directories(${target} PRIVATE ${OPENMESH_INCLUDE_DIRS})
	target_link_libraries(${target} PRIVATE ${OPENMESH_LIBRARIES})

    target_link_libraries(${target} PRIVATE d3d11.lib d3dcompiler.lib dxgi.lib dxguid.lib)
endmacro()

add_executable (Nome3 ${PG2_SOURCES} ${PG2_GENERATED_SOURCES})
config_playground2(Nome3)
add_executable (Nome3_test ${PG2_SOURCES} ${PG2_GENERATED_SOURCES} ${PG2_TEST_SOURCES})
config_playground2(Nome3_test)
target_compile_definitions(Nome3_test PRIVATE DISABLE_MAIN_FOR_TESTS)
