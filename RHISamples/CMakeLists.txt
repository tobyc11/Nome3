set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

add_executable(WindowLess WindowLess.cpp)
target_link_libraries(WindowLess PRIVATE RHI)
target_compile_definitions(WindowLess PRIVATE -DAPP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")


add_executable(TriangleDemo TriangleDemo.cpp)
target_link_libraries(TriangleDemo PRIVATE RHI)
target_compile_definitions(TriangleDemo PRIVATE -DAPP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

if(TARGET SDL2::SDL2)
    target_link_libraries(TriangleDemo PRIVATE SDL2::SDL2 SDL2::SDL2main)
else()
    target_include_directories(TriangleDemo PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(TriangleDemo PRIVATE ${SDL2_LIBRARIES})
endif()
