project(mesh-simplification)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.cc include/*.h)
add_library(meshark ${SOURCES})
target_include_directories(meshark PUBLIC include)
target_link_libraries(meshark PUBLIC glm)

add_executable(simplify apps/simplify.cc)
target_link_libraries(simplify meshark)