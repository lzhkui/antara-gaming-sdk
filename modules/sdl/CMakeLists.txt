message(STATUS "SDL WRAPPER ENABLED")
#add_library(sdl_shared_resources INTERFACE)
#target_include_directories(sdl_shared_resources INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(sdl_shared_resources INTERFACE antara::ecs antara::config antara::external_sdl)
#add_library(antara::sdl_shared_resources ALIAS sdl_shared_resources)

#add_library(antara_sdl_graphic STATIC)
#target_sources(antara_sdl_graphic PRIVATE antara/gaming/sdl/graphic.system.cpp)
#target_link_libraries(antara_sdl_graphic PUBLIC antara::sdl_shared_resources)
#add_library(antara::sdl_graphic ALIAS antara_sdl_graphic)

#add_library(antara_sdl_input STATIC)
#target_sources(antara_sdl_input PRIVATE antara/gaming/sdl/input.system.cpp)
#target_link_libraries(antara_sdl_input PUBLIC antara::sdl_shared_resources)
#add_library(antara::sdl_input ALIAS antara_sdl_input)

add_library(antara_sdl STATIC)
target_include_directories(antara_sdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(antara_sdl PUBLIC antara::ecs antara::config antara::external_sdl)
target_sources(antara_sdl PRIVATE antara/gaming/sdl/graphic.system.cpp antara/gaming/sdl/input.system.cpp
        antara/gaming/sdl/sdl.opengl.image.loading.cpp)
add_library(antara::sdl ALIAS antara_sdl)